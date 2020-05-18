#version 450

layout (local_size_x = 32, local_size_y = 32) in;

layout (binding = 10) uniform writeonly image2D render_texture;
layout (binding = 11) uniform sampler3D normal_texture;
layout (binding = 12) uniform sampler3D colour_texture;

// constants
const uint node_empty_flag = 1 << 24;
const uint node_unused_flag = 1 << 25;
const uint node_child_mask = 0xFFFF;

const float sqrt3 = 1.73205080757;
const uint max_depth = 16;

// these ones could be push constants hypothetically
const float f = 1.0;
const int max_steps = 64;
const float epsilon = 1.0 / 256.0;

// types 
struct ray_t {
    vec3 x;
    vec3 d;
};

struct intersection_t {
    bool hit;
    vec3 x;
    vec3 normal;
    vec3 texture_coord;
};

// push constants
layout( push_constant ) uniform push_constants {
    uvec2 window_size;
    float render_distance;
    uint current_frame;

    vec3 camera_position;
    float phi_initial;        

    vec3 camera_right;
    float dummy3;

    vec3 camera_up;
    float dummy4;
} pc;

// input buffers
struct substance_t {
    vec3 c;
    int root;

    float r;
    uint q;
    uint _2;
    uint id;
};

layout (binding = 1) buffer input_buffer { 
    substance_t substances[gl_WorkGroupSize.x * gl_WorkGroupSize.y];
    uvec2 octree[]; 
} input_data;

// output buffers
struct request_t {
    vec3 c;
    uint depth;

    uint child;
    uint unused;
    uint substanceID;
    uint status;
};

layout (binding = 2) buffer request_buffer { request_t requests[]; } requests;

// work group data that is persistent between frames
struct persistent_t {
    vec3 v_min;
    float _1;

    vec3 v_max;
    float _2;
};

layout (binding = 3) buffer persistent_buffer { persistent_t data[]; } persistent;

// shared memory
shared uint vacant_node_index;

shared substance_t substances[32];
shared uvec2 octree[gl_WorkGroupSize.x * gl_WorkGroupSize.y];
shared bool hitmap[gl_WorkGroupSize.x * gl_WorkGroupSize.y / 8];

shared uint substance_counter[gl_WorkGroupSize.x * gl_WorkGroupSize.y];
shared vec3 visibility[gl_WorkGroupSize.x * gl_WorkGroupSize.y];

uint expected_depth(vec3 x){
    return uint(3 * exp(-0.03 * length(x - pc.camera_position)) + 1);
}

float expected_size(vec3 x){
    return pc.render_distance / (1 << expected_depth(x)) / 4;
}

uint work_group_offset(){
    return (gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * gl_WorkGroupSize.x * gl_WorkGroupSize.y;
}

vec4 colour(vec3 t){
    // return vec4(texture(colour_texture, t).xyz, 1.0);
    return vec4(0.9, 0.4, 0.6, 1.0);
}

vec3 rotate(vec4 q, vec3 x){
    float wx = q.x * q.y;
    float wy = q.x * q.z;
    float wz = q.x * q.w;
    
    float xx = q.y * q.y;
    float xy = q.y * q.z;
    float xz = q.y * q.w;

    float yy = q.z * q.z;
    float yz = q.z * q.w;

    float zz = q.w * q.w;

    return vec3(
        (0.5 - yy - zz) * x.x + (xy - wz) * x.y + (xz + wy) * x.z,
        (xy + wz) * x.x + (0.5 - xx - zz) * x.y + (yz + wx) * x.z,
        (xz - wy) * x.x + (yz - wx) * x.y + (0.5 - xx - yy) * x.z
    ) * 2;
}

float phi_s(ray_t r, substance_t sub, uint expected_depth, out vec3 normal, out vec3 t){
    vec3 c = vec3(0);
    vec3 c_prev = c;
    vec3 s = vec3(sub.r);

    uint i;
    uint depth = 0;
    uint next = sub.root;

    vec4 q = vec4(sub.q & 0xFF, (sub.q >> 8) & 0xFF, (sub.q >> 16) & 0xFF, sub.q >> 24) - 127.5;
    q = normalize(q);

    r.x -= sub.c;
    r.x = rotate(q, r.x);

    // check against outside bounds of aabb
    vec3 y = abs(r.x) - s;
    float phi_aabb = length(max(y, 0)) + min(max(y.x, max(y.y, y.z)), 0) + epsilon;

    // perform octree lookup for relevant node
    for (; depth <= expected_depth && (depth == 0 || next != 0) && (octree[next].x & node_unused_flag) == 0; depth++){
        i = next | uint(dot(step(c, r.x), vec3(1, 2, 4)));
        hitmap[i / 8] = true;
        next = octree[i].x & node_child_mask;
        c_prev = c;
        c += sign(r.x - c) * (s /= 2);
    }

    uvec2 node = octree[i];

    // if necessary, request more data from CPU
    // TODO: move this to postrender(), remove atomic operation
    bool should_request = depth <= expected_depth && (node.x & node_empty_flag) == 0 && (node.x & node_child_mask) == 0;
    uint child = atomicAnd(vacant_node_index, mix(~0, 0, should_request)); 
    request_t request = request_t(c, depth, 0, 0, sub.id, 1);
    if (should_request && child != 0){
        input_data.octree[i + work_group_offset()].x |= child;
        request.child = child + work_group_offset();
        requests.requests[uint(dot(gl_WorkGroupID.xy, vec2(1, gl_NumWorkGroups.x)))] = request;
    }

    // calculate distance to intersect plane
    vec3 n = vec3(node.y & 0xFF, (node.y >> 8) & 0xFF, (node.y >> 16) & 0xFF) / 127.5 - 1;
    float e = dot(c - r.x, n) - (float(node.y) / 1235007097.17 - sqrt3) * s.x;
    float phi_plane = min(0, e) / dot(r.d, n);

    // calculate texture coordinate
    t = (r.x - c_prev + s * 4) / (s * 8);
    t += vec3(
        (i + work_group_offset()) % (gl_WorkGroupSize.x * gl_NumWorkGroups.x) / 8,
        (i + work_group_offset()) / (gl_WorkGroupSize.y * gl_NumWorkGroups.y),
        0
    );
    t /= vec3(gl_WorkGroupSize.xy * gl_NumWorkGroups.xy / vec2(8, 1), 1);

    normal = rotate(vec4(q.x, -q.yzw), normalize(texture(normal_texture, t).xyz - 0.5));
    
    // return distance value appropriate for case
    bool hit = (node.x & node_empty_flag) == 0 && phi_plane >= 0;
    return mix(mix(s.x, phi_plane, hit), phi_aabb, phi_aabb > epsilon);
}

intersection_t raycast(ray_t r, inout vec3 v_min, inout vec3 v_max){
    bool hit = false;
    vec3 normal, texture_coord;
    uint steps;

    v_min = min(r.x, v_min);
    v_max = max(r.x, v_max);

    for (steps = 0; !hit && steps < max_steps; steps++){
        uint expected_depth = expected_depth(r.x);
        float phi = pc.render_distance;
        for (uint substanceID = 0; !hit && substanceID < 3; substanceID++){
            phi = min(phi, phi_s(r, substances[substanceID], expected_depth, normal, texture_coord));
            hit = hit || phi < epsilon;
        }
        r.x += r.d * phi;
    }

    v_min = min(r.x, v_min);
    v_max = max(r.x, v_max);

    return intersection_t(hit && steps < max_steps, r.x, normal, texture_coord);
}

float shadow(vec3 l, vec3 p, inout vec3 v_min, inout vec3 v_max){
    return 1;
    // intersection_t i = raycast(ray_t(l, normalize(p - l)), v_min, v_max);
    // return float(length(i.x - p) < epsilon * 2);
}

vec4 light(vec3 light_p, vec3 x, vec3 n, vec3 t, inout vec3 v_min, inout vec3 v_max){
    vec4 colour = vec4(50);
    float kd = 0.5;
    float ks = 0.76;
    float shininess = 32;

    // attenuation
    float dist = length(light_p - x);
    float attenuation = 1.0 / (dist * dist);

    //ambient 
    vec4 a = vec4(0.5, 0.5, 0.5, 1.0);

    //shadows
    float shadow = shadow(light_p, x, v_min, v_max);

    //diffuse
    vec3 l = normalize(light_p - x);

    vec4 d = kd * dot(l, n) * colour;

    //specular
    vec3 v = x - pc.camera_position;
    vec3 right = pc.camera_right;
    vec3 u = pc.camera_up;

    // TODO: not sure about order of cross product here??
    v = vec3(dot(v, right), dot(v, u), dot(v, cross(u, right))); 
    v = normalize(v);

    vec3 r = reflect(l, n);
    vec4 s = ks * pow(max(dot(r, v), epsilon), shininess) * colour;
    return a + (d + s) * attenuation * shadow;
}

vec4 sky(){
    return vec4(0.5, 0.7, 0.9, 1.0);
}

void render(inout vec3 v_min, inout vec3 v_max){
    vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2(gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    uv = uv * 2.0 - 1.0;
    uv.y *= -1;
    
    vec3 up = pc.camera_up;
    vec3 right = pc.camera_right;
    vec3 forward = cross(right, up);
    vec3 d = normalize(forward * f + right * uv.x + up * uv.y);

    ray_t r = ray_t(pc.camera_position + pc.phi_initial * d, d);
    intersection_t i = raycast(r, v_min, v_max);

    vec4 hit_colour = colour(i.texture_coord) * light(vec3(-3, 3, -3), i.x, i.normal, i.texture_coord, v_min, v_max);
    imageStore(render_texture, ivec2(gl_GlobalInvocationID.xy), mix(sky(), hit_colour, i.hit));
}

bool is_visible(substance_t substance){
    return true;
}

void prerender(uint i){
    // clear shared variables
    vacant_node_index = 0;
    hitmap[i / 8] = false;

    // load octree from global memory into shared memory
    octree[i] = input_data.octree[i + work_group_offset()];

    // submit free nodes to request queue
    if ((i & 7) == 0 && (octree[i].x & node_unused_flag) != 0){
        vacant_node_index = i; 
    }

    // visibility check on substances and load into shared memory
    substance_t s = input_data.substances[i];
    bool visible_substance = s.id != ~0 && is_visible(s);
    substance_counter[i] = uint(visible_substance);
    
    if ((i &   1) != 0) substance_counter[i] += substance_counter[i &   ~1      ];    
    if ((i &   2) != 0) substance_counter[i] += substance_counter[i &   ~2 |   1];    
    if ((i &   4) != 0) substance_counter[i] += substance_counter[i &   ~4 |   3];    
    if ((i &   8) != 0) substance_counter[i] += substance_counter[i &   ~8 |   7];    
    if ((i &  16) != 0) substance_counter[i] += substance_counter[i &  ~16 |  15];    
    if ((i &  32) != 0) substance_counter[i] += substance_counter[i &  ~32 |  31];    
    if ((i &  64) != 0) substance_counter[i] += substance_counter[i &  ~64 |  63];    
    if ((i & 128) != 0) substance_counter[i] += substance_counter[i & ~128 | 127];    
    if ((i & 256) != 0) substance_counter[i] += substance_counter[i & ~256 | 255];    
    if ((i & 512) != 0) substance_counter[i] += substance_counter[           511];    
 
    if (visible_substance && substance_counter[i] <= 32){
        substances[substance_counter[i] - 1] = s;
    }
}

void postrender(uint i, vec3 v_min, vec3 v_max){
    // cull nodes that havent been seen this frame
    uint c = (octree[i].x & node_child_mask);
    if (!hitmap[c / 8]){
        input_data.octree[i + work_group_offset()].x &= ~node_child_mask;
        input_data.octree[c + work_group_offset()].x |= node_unused_flag;
    }

    // fold over min and max aabb values and store
    uint work_group_id = gl_WorkGroupID.x * gl_NumWorkGroups.x + gl_WorkGroupID.y;

    visibility[i] = v_min;
    if ((i & 0x001) == 0) visibility[i] = min(visibility[i], visibility[i +   1]);
    if ((i & 0x003) == 0) visibility[i] = min(visibility[i], visibility[i +   2]);
    if ((i & 0x007) == 0) visibility[i] = min(visibility[i], visibility[i +   4]);
    if ((i & 0x00F) == 0) visibility[i] = min(visibility[i], visibility[i +   8]);
    if ((i & 0x01F) == 0) visibility[i] = min(visibility[i], visibility[i +  16]);
    if ((i & 0x03F) == 0) visibility[i] = min(visibility[i], visibility[i +  32]);
    if ((i & 0x07F) == 0) visibility[i] = min(visibility[i], visibility[i +  64]);
    if ((i & 0x0FF) == 0) visibility[i] = min(visibility[i], visibility[i + 128]);
    if ((i & 0x1FF) == 0) visibility[i] = min(visibility[i], visibility[i + 256]);
    if (i == 0) persistent.data[work_group_id].v_min = min(visibility[0], visibility[512]);

    visibility[i] = v_max;
    if ((i & 0x001) == 0) visibility[i] = max(visibility[i], visibility[i +   1]);
    if ((i & 0x003) == 0) visibility[i] = max(visibility[i], visibility[i +   2]);
    if ((i & 0x007) == 0) visibility[i] = max(visibility[i], visibility[i +   4]);
    if ((i & 0x00F) == 0) visibility[i] = max(visibility[i], visibility[i +   8]);
    if ((i & 0x01F) == 0) visibility[i] = max(visibility[i], visibility[i +  16]);
    if ((i & 0x03F) == 0) visibility[i] = max(visibility[i], visibility[i +  32]);
    if ((i & 0x07F) == 0) visibility[i] = max(visibility[i], visibility[i +  64]);
    if ((i & 0x0FF) == 0) visibility[i] = max(visibility[i], visibility[i + 128]);
    if ((i & 0x1FF) == 0) visibility[i] = max(visibility[i], visibility[i + 256]);
    if (i == 0) persistent.data[work_group_id].v_max = max(visibility[0], visibility[512]);
}

void main(){
    uint i = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
    vec3 v_min = pc.camera_position;
    vec3 v_max = pc.camera_position;

    prerender(i);

    barrier();
    render(v_min, v_max);
    barrier();

    postrender(i, v_min, v_max);
}
