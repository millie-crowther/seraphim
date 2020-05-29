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
    float distance;
};

// push constants
layout( push_constant ) uniform push_constants {
    uvec2 window_size;
    float render_distance;
    uint current_frame;

    vec3 camera_position;
    float phi_initial;        

    vec3 eye_right;
    float focal_depth;

    vec3 eye_up;
    float dummy4;
} pc;

// input buffers
struct substance_data_t {
    vec3 c;
    int root;

    float r;
    uint q;
    uint _2;
    uint id;
};

struct substance_t {
    vec3 c;
    int root;

    float r;
    uint id;

    mat3 rotation;
    mat3 inverse;
};

// output buffers
struct request_t {
    vec3 c;
    float size;

    uint child;
    uint parent;
    uint substanceID;
    uint status;
};

layout (binding = 1) buffer octree_buffer    { uvec2            data[]; } octree_global;
layout (binding = 2) buffer request_buffer   { request_t        data[]; } requests;
layout (binding = 3) buffer depth_buffer     { float            data[]; } depth;
layout (binding = 4) buffer substance_buffer { substance_data_t data[]; } substance;

// shared memory
shared uint vacant_node;
shared uint chosen_request;
shared uint substances_visible;

shared substance_t substances[32];
shared uvec2 octree[gl_WorkGroupSize.x * gl_WorkGroupSize.y];
shared bool hitmap[gl_WorkGroupSize.x * gl_WorkGroupSize.y / 8];
shared uvec4 workspace[gl_WorkGroupSize.x * gl_WorkGroupSize.y];

vec2 uv(){
    vec2 uv = vec2(gl_GlobalInvocationID.xy) / (gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    uv = uv * 2.0 - 1.0;
    uv.y *= -float(gl_NumWorkGroups.y) / gl_NumWorkGroups.x;
    return uv;
}

float expected_size(vec3 x){
    return 0.075 * (1 + length((x - pc.camera_position) / 10) + length(uv() / 2));
}

uint work_group_offset(){
    return (gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * gl_WorkGroupSize.x * gl_WorkGroupSize.y;
}

vec4 colour(vec3 t){
    return vec4(texture(colour_texture, t).xyz, 1.0);
}

mat3 get_mat(vec4 q){
    float wx = q.x * q.y;
    float wy = q.x * q.z;
    float wz = q.x * q.w;
    
    float xx = q.y * q.y;
    float xy = q.y * q.z;
    float xz = q.y * q.w;

    float yy = q.z * q.z;
    float yz = q.z * q.w;

    float zz = q.w * q.w;

    vec3 a = vec3(
        0.5 - yy - zz,
        xy - wz,
        xz + wy
    );

    vec3 b = vec3(
        xy + wz,
        0.5 - xx - zz,
        yz + wx
    );

    vec3 c = vec3(
        xz - wy,
        yz - wx,
        0.5 - xx - yy
    );

    return mat3(a, b, c) * 2;
}

float phi_s(ray_t r, substance_t sub, float expected_size, out vec3 normal, out vec3 t, inout request_t request){
    vec3 c = vec3(0);
    vec3 c_prev = c;
    vec3 s = vec3(sub.r);

    uint i = ~0;
    uint next = sub.root;

    r.x -= sub.c;
    r.x = sub.rotation * r.x;

    // check against outside bounds of aabb
    vec3 y = abs(r.x) - s;
    float phi_aabb = length(max(y, 0)) + min(max(y.x, max(y.y, y.z)), 0) + epsilon;

    // perform octree lookup for relevant node
    while (s.x >= expected_size && next != node_child_mask && (octree[next].x & node_unused_flag) == 0){
        i = next | uint(dot(step(c, r.x), vec3(1, 2, 4)));
        hitmap[i / 8] = true;
        next = octree[i].x & node_child_mask;
        c_prev = c;
        c += sign(r.x - c) * (s /= 2);
    }

    uvec2 node = octree[i];

    // if necessary, request more data from CPU
    bool should_request = s.x >= expected_size && (node.x & node_empty_flag) == 0 && next == node_child_mask && request.status == 0;
    if (should_request) request = request_t(c, s.x, 0, i, sub.id, 1);
    
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

    normal = sub.inverse * normalize(texture(normal_texture, t).xyz - 0.5);
    
    // return distance value appropriate for case
    bool hit = (node.x & node_empty_flag) == 0 && phi_plane >= 0;
    return mix(mix(s.x, phi_plane, hit), phi_aabb, phi_aabb > epsilon);
}

intersection_t raycast(ray_t r, inout request_t request){
    bool hit = false;
    uint steps;
    intersection_t i;
    i.hit = false;
    i.distance = 0;

    for (steps = 0; !i.hit && steps < max_steps; steps++){
        float expected_size = expected_size(r.x);
        float phi = pc.render_distance;
        for (uint substanceID = 0; !i.hit && substanceID < substances_visible; substanceID++){
            phi = min(phi, phi_s(r, substances[substanceID], expected_size, i.normal, i.texture_coord, request));
            i.hit = i.hit || phi < epsilon;
        }
        r.x += r.d * phi;
        i.distance += phi;
    }

    i.x = r.x;
    return i;
}

float shadow(vec3 l, vec3 p, inout request_t request){
    // return 1;
    intersection_t i = raycast(ray_t(l, normalize(p - l)), request);
    return float(length(i.x - p) < epsilon * 10);
}

vec4 light(vec3 light_p, vec3 x, vec3 n, vec3 t, inout request_t request){
    vec4 colour = vec4(50);
    float kd = 0.5;
    float ks = 0.76;
    float shininess = 32;

    // attenuation
    float dist = length(light_p - x);
    float attenuation = 1.0 / (dist * dist);

    //ambient 
    vec4 a = vec4(0.25, 0.25, 0.25, 1.0);

    //shadows
    float shadow = shadow(light_p, x, request);

    //diffuse
    vec3 l = normalize(light_p - x);

    vec4 d = kd * max(epsilon, dot(l, n)) * colour;

    //specular
    vec3 v = x - pc.camera_position;
    vec3 right = pc.eye_right;
    vec3 u = pc.eye_up;

    v = vec3(dot(v, right), dot(v, u), dot(v, cross(u, right))); 
    v = normalize(v);

    vec3 r = reflect(l, n);
    vec4 s = ks * pow(max(dot(r, v), epsilon), shininess) * colour;
    return a + (d + s) * attenuation * shadow;
}

vec4 sky(){
    return vec4(0.5, 0.7, 0.9, 1.0);
}

request_t render(uint i){
    vec2 uv = uv();
    vec3 up = pc.eye_up;
    vec3 right = pc.eye_right;
    vec3 forward = cross(right, up);
    vec3 d = normalize(forward * pc.focal_depth + right * uv.x + up * uv.y);

    request_t request;
    request.status = 0;

    ray_t r = ray_t(pc.camera_position, d);
    intersection_t intersection = raycast(r, request);

    vec4 hit_colour = colour(intersection.texture_coord) * light(vec3(-3, 3, -3), intersection.x, intersection.normal, intersection.texture_coord, request);
    imageStore(render_texture, ivec2(gl_GlobalInvocationID.xy), mix(sky(), hit_colour, intersection.hit));

    return request;
}

bool is_visible(substance_data_t sub){
    vec3 x = sub.c - pc.camera_position;
    float d = dot(x, cross(pc.eye_right, pc.eye_up));
    vec2 t = vec2(dot(x, pc.eye_right), dot(x, pc.eye_up)) / d * pc.focal_depth;
    t.y *= -float(gl_NumWorkGroups.x) / gl_NumWorkGroups.y;

    ivec2 image_x = ivec2((t + 1) * gl_NumWorkGroups.xy * gl_WorkGroupSize.xy) / 2;
    float r = sub.r / d * pc.focal_depth * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
    ivec2 c = ivec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy + gl_WorkGroupSize.xy / 2);
    ivec2 diff = max(ivec2(0), abs(c - image_x) - ivec2(gl_WorkGroupSize.xy / 2));

    return length(diff) < r;
}

void prerender(uint i, uint work_group_id){
    // clear shared variables
    if (i == 0){
        vacant_node = 0;
        chosen_request = ~0;
    }
    hitmap[i / 8] = false;

    // load octree from global memory into shared memory
    octree[i] = octree_global.data[i + work_group_offset()];

    // submit free nodes to request queue
    if ((i & 7) == 0 && (octree[i].x & node_unused_flag) != 0){
        vacant_node = i; 
    }

    // visibility check on substances and load into shared memory
    barrier();
    substance_data_t s = substance.data[i];
    bool visible_substance = s.id != ~0 && is_visible(s);
    workspace[i / 4][i % 4] = uint(visible_substance);
    barrier();

    uvec4 x = workspace[i >> 2];
    workspace[i >> 2] = uvec4(x.x, x.x + x.y, x.x + x.y + x.z, x.x + x.y + x.z + x.w);

    barrier();
    if ((i &   1) != 0) workspace[i] += workspace[i &  ~1      ].w;    
    barrier();
    if ((i &   2) != 0) workspace[i] += workspace[i &  ~2 |   1].w;    
    barrier();
    if ((i &   4) != 0) workspace[i] += workspace[i &  ~4 |   3].w;    
    barrier();
    if ((i &   8) != 0) workspace[i] += workspace[i &  ~8 |   7].w;    
    barrier();
    if ((i &  16) != 0) workspace[i] += workspace[i & ~16 |  15].w;    
    barrier();
    if ((i &  32) != 0) workspace[i] += workspace[i & ~32 |  31].w;    
    barrier();
    if ((i &  64) != 0) workspace[i] += workspace[i & ~64 |  63].w;    
    barrier();
    if ((i & 128) != 0) workspace[i] += workspace[          127].w;    
    barrier();

    if (visible_substance && workspace[i / 4][i % 4] <= gl_WorkGroupSize.x){
        vec4 q = vec4(s.q & 0xFF, (s.q >> 8) & 0xFF, (s.q >> 16) & 0xFF, s.q >> 24) - 127.5;
        q = normalize(q);
        substances[workspace[i / 4][i % 4] - 1] = substance_t(
            s.c, s.root, s.r, s.id, get_mat(q), get_mat(vec4(q.x, -q.yzw))
        );
    }

    substances_visible = min(workspace[255].w, gl_WorkGroupSize.x);
}

void postrender(uint i, request_t request){
    // arbitrate and submit request
    if ((i & 0x001) == 0) workspace[i] = min(workspace[i], workspace[i +   1]);
    if ((i & 0x003) == 0) workspace[i] = min(workspace[i], workspace[i +   2]);
    if ((i & 0x007) == 0) workspace[i] = min(workspace[i], workspace[i +   4]);
    if ((i & 0x00F) == 0) workspace[i] = min(workspace[i], workspace[i +   8]);
    if ((i & 0x01F) == 0) workspace[i] = min(workspace[i], workspace[i +  16]);
    if ((i & 0x03F) == 0) workspace[i] = min(workspace[i], workspace[i +  32]);
    if ((i & 0x07F) == 0) workspace[i] = min(workspace[i], workspace[i +  64]);
    
    uvec4 m = min(workspace[0], workspace[128]);
    if (i == min(min(m.x, m.y), min(m.z, m.w))){
        octree_global.data[request.parent + work_group_offset()].x &= ~node_child_mask | vacant_node;

        request.child = vacant_node + work_group_offset();
        requests.data[uint(dot(gl_WorkGroupID.xy, vec2(1, gl_NumWorkGroups.x)))] = request;
    }

    // cull nodes that havent been seen this frame
    uint c = (octree[i].x & node_child_mask);
    if (c != node_child_mask && !hitmap[c >> 3]){
        octree_global.data[i + work_group_offset()].x |= node_child_mask;
        octree_global.data[c + work_group_offset()].x |= node_unused_flag;
    }
}

void main(){
    uint work_group_id = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
    uint i = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
    vec3 v_min = pc.camera_position;
    vec3 v_max = pc.camera_position;

    prerender(i, work_group_id);

    barrier();
    request_t request = render(i);

    // submit request before barrier so that arbitration can start immediately afterwards
    workspace[i / 4][i % 4] = mix(~0, i, request.status != 0 && vacant_node != 0);
    barrier();

    postrender(i, request);
}
