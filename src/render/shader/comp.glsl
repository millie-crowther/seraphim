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
    float focal_depth;

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
    uint parent;
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
shared uint vacant_node;
shared uint chosen_request;
shared uint substances_visible;
shared vec3 volume_min;
shared vec3 volume_max;

shared substance_t substances[32];
shared uvec2 octree[gl_WorkGroupSize.x * gl_WorkGroupSize.y];
shared bool hitmap[gl_WorkGroupSize.x * gl_WorkGroupSize.y / 8];

shared uint workspace[gl_WorkGroupSize.x * gl_WorkGroupSize.y];
shared uvec4 uworkspace[gl_WorkGroupSize.x * gl_WorkGroupSize.y / 4];
shared vec3 visibility[gl_WorkGroupSize.x * gl_WorkGroupSize.y];

vec2 uv(){
    vec2 uv = vec2(gl_GlobalInvocationID.xy) / (gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    uv = uv * 2.0 - 1.0;
    uv.y *= -float(gl_NumWorkGroups.y) / gl_NumWorkGroups.x;
    return uv;
}

float expected_size(vec3 x){
    return 0.075 * (1 + length(
        (x - pc.camera_position) / 10 + 
        vec3(uv(), 0) / 2
    ));
}

uint work_group_offset(){
    return (gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * gl_WorkGroupSize.x * gl_WorkGroupSize.y;
}

vec4 colour(vec3 t){
    return vec4(texture(colour_texture, t).xyz, 1.0);
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

float phi_s(ray_t r, substance_t sub, float expected_size, out vec3 normal, out vec3 t, inout request_t request){
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
    for (; s.x >= expected_size && (depth == 0 || next != 0) && (octree[next].x & node_unused_flag) == 0; depth++){
        i = next | uint(dot(step(c, r.x), vec3(1, 2, 4)));
        hitmap[i / 8] = true;
        next = octree[i].x & node_child_mask;
        c_prev = c;
        c += sign(r.x - c) * (s /= 2);
    }

    uvec2 node = octree[i];

    // if necessary, request more data from CPU
    // TODO: move this to postrender(), remove atomic operation
    bool should_request = s.x >= expected_size && (node.x & node_empty_flag) == 0 && (node.x & node_child_mask) == 0 && request.status == 0;
    if (should_request) request = request_t(c, depth, 0, i, sub.id, 1);
    

    // calculate distance to intersect plane
    vec3 n = vec3(node.y & 0xFF, (node.y >> 8) & 0xFF, (node.y >> 16) & 0xFF) / 127.5 - 1;
    float e = dot(c - r.x, n) - (float(node.y) / 1235007097.17 - sqrt3) * s.x;
    float phi_plane = min(0, e) / dot(r.d, n);

    // calculate texture coordinate
    t = (r.x - c_prev + s * 4) / (s * 8);
    t += vec3(
        (i + work_group_offset()) % (gl_WorkGroupSize.x * gl_NumWorkGroups.x) / 8,
        (i + work_group_offset()) / (gl_WorkGroupSize.x * gl_NumWorkGroups.x),
        0
    );
    t /= vec3(gl_WorkGroupSize.xy * gl_NumWorkGroups.xy / vec2(8, 1), 1);

    normal = rotate(vec4(q.x, -q.yzw), normalize(texture(normal_texture, t).xyz - 0.5));
    
    // return distance value appropriate for case
    bool hit = (node.x & node_empty_flag) == 0 && phi_plane >= 0;
    return mix(mix(s.x, phi_plane, hit), phi_aabb, phi_aabb > epsilon);
}

intersection_t raycast(ray_t r, inout vec3 v_min, inout vec3 v_max, inout request_t request){
    bool hit = false;
    vec3 normal, texture_coord;
    uint steps;

    v_min = min(r.x, v_min);
    v_max = max(r.x, v_max);

    for (steps = 0; !hit && steps < max_steps; steps++){
        float expected_size = expected_size(r.x);
        float phi = pc.render_distance;
        for (uint substanceID = 0; !hit && substanceID < substances_visible; substanceID++){
            phi = min(phi, phi_s(r, substances[substanceID], expected_size, normal, texture_coord, request));
            hit = hit || phi < epsilon;
        }
        r.x += r.d * phi;
    }

    v_min = min(r.x, v_min);
    v_max = max(r.x, v_max);

    return intersection_t(hit && steps < max_steps, r.x, normal, texture_coord);
}

float shadow(vec3 l, vec3 p, inout vec3 v_min, inout vec3 v_max, inout request_t request){
    return 1;
    // intersection_t i = raycast(ray_t(l, normalize(p - l)), v_min, v_max, request);
    // return float(length(i.x - p) < epsilon * 2);
}

vec4 light(vec3 light_p, vec3 x, vec3 n, vec3 t, inout vec3 v_min, inout vec3 v_max, inout request_t request){
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
    float shadow = shadow(light_p, x, v_min, v_max, request);

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

request_t render(inout vec3 v_min, inout vec3 v_max){
    vec2 uv = uv();
    vec3 up = pc.camera_up;
    vec3 right = pc.camera_right;
    vec3 forward = cross(right, up);
    vec3 d = normalize(forward * pc.focal_depth + right * uv.x + up * uv.y);

    request_t request;
    request.status = 0;

    ray_t r = ray_t(pc.camera_position + pc.phi_initial * d, d);
    intersection_t i = raycast(r, v_min, v_max, request);

    vec4 hit_colour = colour(i.texture_coord) * light(vec3(-3, 3, -3), i.x, i.normal, i.texture_coord, v_min, v_max, request);
    imageStore(render_texture, ivec2(gl_GlobalInvocationID.xy), mix(sky(), hit_colour, i.hit));

    return request;
}

bool is_visible(substance_t sub){
    // TODO: problem when eye is close to the substance
    float p = length(vec3(sub.r));

    vec3 lt = vec3(lessThanEqual(sub.c, volume_min)) * abs(sub.c - volume_min);
    float ltd = dot(lt, vec3(1));
    
    vec3 gt = vec3(greaterThanEqual(sub.c, volume_max)) * abs(sub.c - volume_max);
    float gtd = dot(gt, vec3(1));

    p -= ltd + gtd;

    // return p > -epsilon;
    return true;
}

void prerender(uint i, uint work_group_id){
    // clear shared variables
    if (i == 0){
        vacant_node = 0;
        chosen_request = ~0;
        
        volume_min = persistent.data[work_group_id].v_min;
        volume_max = persistent.data[work_group_id].v_max;
    }
    hitmap[i / 8] = false;

    // load octree from global memory into shared memory
    octree[i] = input_data.octree[i + work_group_offset()];

    // submit free nodes to request queue
    if ((i & 7) == 0 && (octree[i].x & node_unused_flag) != 0){
        vacant_node = i; 
    }

    // visibility check on substances and load into shared memory
    barrier();
    substance_t s = input_data.substances[i];
    bool visible_substance = s.id != ~0 && is_visible(s);
    workspace[i] = uint(visible_substance);
    
    if ((i &   1) != 0) workspace[i] = workspace[i] + workspace[i &   ~1      ];    
    barrier();
    if ((i &   2) != 0) workspace[i] = workspace[i] + workspace[i &   ~2 |   1];    
    barrier();
    if ((i &   4) != 0) workspace[i] = workspace[i] + workspace[i &   ~4 |   3];    
    barrier();
    if ((i &   8) != 0) workspace[i] = workspace[i] + workspace[i &   ~8 |   7];    
    barrier();
    if ((i &  16) != 0) workspace[i] = workspace[i] + workspace[i &  ~16 |  15];    
    barrier();
    if ((i &  32) != 0) workspace[i] = workspace[i] + workspace[i &  ~32 |  31];    
    barrier();
    if ((i &  64) != 0) workspace[i] = workspace[i] + workspace[i &  ~64 |  63];    
    barrier();
    if ((i & 128) != 0) workspace[i] = workspace[i] + workspace[i & ~128 | 127];    
    barrier();
    if ((i & 256) != 0) workspace[i] = workspace[i] + workspace[i & ~256 | 255];    
    barrier();
    if ((i & 512) != 0) workspace[i] = workspace[i] + workspace[           511];    
 
    if (visible_substance && workspace[i] <= gl_WorkGroupSize.x){
        substances[workspace[i] - 1] = s;
    }

    substances_visible = min(workspace[1023], gl_WorkGroupSize.x);
}

void postrender(uint i, uint work_group_id, vec3 v_min, vec3 v_max, request_t request){
    // arbitrate and submit request
    if ((i & 0x001) == 0) uworkspace[i] = min(uworkspace[i], uworkspace[i +   1]);
    if ((i & 0x003) == 0) uworkspace[i] = min(uworkspace[i], uworkspace[i +   2]);
    if ((i & 0x007) == 0) uworkspace[i] = min(uworkspace[i], uworkspace[i +   4]);
    if ((i & 0x00F) == 0) uworkspace[i] = min(uworkspace[i], uworkspace[i +   8]);
    if ((i & 0x01F) == 0) uworkspace[i] = min(uworkspace[i], uworkspace[i +  16]);
    if ((i & 0x03F) == 0) uworkspace[i] = min(uworkspace[i], uworkspace[i +  32]);
    if ((i & 0x07F) == 0) uworkspace[i] = min(uworkspace[i], uworkspace[i +  64]);
    
    uvec4 m = min(uworkspace[0], uworkspace[128]);
    if (i == min(min(m.x, m.y), min(m.z, m.w))){
        input_data.octree[request.parent + work_group_offset()].x |= vacant_node;
        request.child = vacant_node + work_group_offset();
        requests.requests[uint(dot(gl_WorkGroupID.xy, vec2(1, gl_NumWorkGroups.x)))] = request;
    }

    // cull nodes that havent been seen this frame
    uint c = (octree[i].x & node_child_mask);
    if (!hitmap[c >> 3]){
        input_data.octree[i + work_group_offset()].x &= ~node_child_mask;
        input_data.octree[c + work_group_offset()].x |= node_unused_flag;
    }

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
    uint work_group_id = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
    uint i = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
    vec3 v_min = pc.camera_position;
    vec3 v_max = pc.camera_position;

    prerender(i, work_group_id);

    barrier();
    request_t request = render(v_min, v_max);

    // submit request before barrier so that arbitration can start immediately afterwards
    uworkspace[i / 4][i % 4] = mix(~0, i, request.status != 0 && vacant_node != 0);
    barrier();

    postrender(i, work_group_id, v_min, v_max, request);
}
