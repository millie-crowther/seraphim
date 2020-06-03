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

struct request_t {
    vec3 c;
    float size;

    uint child;
    uint parent;
    uint substanceID;
    uint status;
};

struct light_t {
    vec3 x;
    float _1;

    vec4 colour;
};

layout (binding = 1) buffer octree_buffer    { uvec2            data[]; } octree_global;
layout (binding = 2) buffer request_buffer   { request_t        data[]; } requests;
layout (binding = 3) buffer depth_buffer     { float            data[]; } depth;
layout (binding = 4) buffer substance_buffer { substance_data_t data[]; } substance;


// shared memory
shared uint vacant_node;
shared uint chosen_request;

shared substance_t substances[gl_WorkGroupSize.x];
shared uint substances_visible;
shared vec3 surface_min;
shared vec3 surface_max;

shared substance_t shadow_substances[gl_WorkGroupSize.x];
shared uint shadow_substances_visible;
shared vec3 lights_min;
shared vec3 lights_max;

shared light_t lights[32];
shared uint lights_visible;

shared uvec2 octree[gl_WorkGroupSize.x * gl_WorkGroupSize.y];
shared bool hitmap[gl_WorkGroupSize.x * gl_WorkGroupSize.y / 8];
shared vec4 workspace[gl_WorkGroupSize.x * gl_WorkGroupSize.y];

vec2 uv(){
    vec2 uv = vec2(gl_GlobalInvocationID.xy) / (gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    uv = uv * 2.0 - 1.0;
    uv.y *= -float(gl_NumWorkGroups.y) / gl_NumWorkGroups.x;
    return uv;
}

float expected_size(vec3 x){
    return 0.075 * (1 + length((x - pc.camera_position) / 10));// + length(uv() / 2));
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

    //
    //  put this in its own function:
    //

    // perform octree lookup for relevant node
    while (s.x >= expected_size && next != node_child_mask && (octree[next].x & node_unused_flag) == 0){
        i = next | uint(dot(step(c, r.x), vec3(1, 2, 4)));
        hitmap[i / 8] = true;
        next = octree[i].x & node_child_mask;
        c_prev = c;
        c += sign(r.x - c) * (s /= 2);
    }

    uvec2 node = octree[i];
    
    // calculate distance to intersect plane
    vec3 n = vec3(node.y & 0xFF, (node.y >> 8) & 0xFF, (node.y >> 16) & 0xFF) / 127.5 - 1;
    float e = dot(c - r.x, n) - (float(node.y) / 1235007097.17 - sqrt3) * s.x;
    float phi_plane = min(0, e) / dot(r.d, n);

    //
    // up to here
    //

    // if necessary, request more data from CPU
    bool should_request = s.x >= expected_size && (node.x & node_empty_flag) == 0 && next == node_child_mask && request.status == 0;
    if (should_request) request = request_t(c, s.x, 0, i, sub.id, 1);

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
    uint n = 32;
    vec3 d = (p - l) / n;
    vec3 rd = normalize(d);
    float phi = 1;
    float expected_size = expected_size(p);
    vec3 _;
    for (uint i = 1; i < n; i++){
        for (uint substanceID = 0; substanceID < shadow_substances_visible; substanceID++){
            phi = min(phi, phi_s(ray_t(l + d * i, rd), shadow_substances[substanceID], expected_size, _, _, request));
        }
    }

    return float(phi > epsilon);
}

vec4 light(light_t light, vec3 x, vec3 n, vec3 t, inout request_t request){
    float shininess = 16;

    // attenuation
    vec3 dist = light.x - x;
    float attenuation = 1.0 / dot(dist, dist);

    //ambient 
    vec4 a = vec4(0.25, 0.25, 0.25, 1.0);

    //shadows
    float shadow = 1;//shadow(light.x, x, request);

    //diffuse
    vec3 l = normalize(light.x - x);
    float d = 0.75 * max(epsilon, dot(l, n));

    //specular
    vec3 v = normalize(-x);

    vec3 h = normalize(l + v);
    float s = 0.4 * pow(max(dot(h, n), 0.0), shininess);

    return a + (d + s) * attenuation * shadow * light.colour;
}

vec4 sky(){
    return vec4(0.5, 0.7, 0.9, 1.0);
}

void reduce_surface_aabb(uint i, bool hit, vec3 x){
    workspace[i] = mix(vec4(pc.render_distance), x.xyzz, hit);

    barrier();
    if ((i & 0x001) == 0) workspace[i] = min(workspace[i], workspace[i +   1]);
    barrier();
    if ((i & 0x003) == 0) workspace[i] = min(workspace[i], workspace[i +   2]);
    barrier();
    if ((i & 0x007) == 0) workspace[i] = min(workspace[i], workspace[i +   4]);
    barrier();
    if ((i & 0x00F) == 0) workspace[i] = min(workspace[i], workspace[i +   8]);
    barrier();
    if ((i & 0x01F) == 0) workspace[i] = min(workspace[i], workspace[i +  16]);
    barrier();
    if ((i & 0x03F) == 0) workspace[i] = min(workspace[i], workspace[i +  32]);
    barrier();
    if ((i & 0x07F) == 0) workspace[i] = min(workspace[i], workspace[i +  64]);
    barrier();
    if ((i & 0x0FF) == 0) workspace[i] = min(workspace[i], workspace[i + 128]);
    barrier();
    if ((i & 0x1FF) == 0) workspace[i] = min(workspace[i], workspace[i + 256]);
    barrier();

    if (i == 0) surface_min = min(workspace[0], workspace[512]).xyz;

    workspace[i] = mix(vec4(-pc.render_distance), x.xyzz, hit);

    barrier();
    if ((i & 0x001) == 0) workspace[i] = max(workspace[i], workspace[i +   1]);
    barrier();
    if ((i & 0x003) == 0) workspace[i] = max(workspace[i], workspace[i +   2]);
    barrier();
    if ((i & 0x007) == 0) workspace[i] = max(workspace[i], workspace[i +   4]);
    barrier();
    if ((i & 0x00F) == 0) workspace[i] = max(workspace[i], workspace[i +   8]);
    barrier();
    if ((i & 0x01F) == 0) workspace[i] = max(workspace[i], workspace[i +  16]);
    barrier();
    if ((i & 0x03F) == 0) workspace[i] = max(workspace[i], workspace[i +  32]);
    barrier();
    if ((i & 0x07F) == 0) workspace[i] = max(workspace[i], workspace[i +  64]);
    barrier();
    if ((i & 0x0FF) == 0) workspace[i] = max(workspace[i], workspace[i + 128]);
    barrier();
    if ((i & 0x1FF) == 0) workspace[i] = max(workspace[i], workspace[i + 256]);
    barrier();

    if (i == 0) surface_max = max(workspace[0], workspace[512]).xyz;
}

bool is_substance_shadow(substance_t s){
    vec3 c_min = min(lights_min, surface_min);
    vec3 c_max = max(lights_max, surface_max);
    vec3 d = max(vec3(0), abs((c_max + c_min) / 2 - s.c) - (c_max - c_min) / 2);
    return length(d) < length(vec3(s.r));
}

uint reduce_to_fit(uint i, bool hit, out uint total){
    workspace[i / 4][i % 4] = uint(hit);
    barrier();

    vec4 x = workspace[i >> 2];
    workspace[i >> 2] = vec4(x.x, x.x + x.y, x.x + x.y + x.z, x.x + x.y + x.z + x.w);

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

    total = min(uint(workspace[255].w), gl_WorkGroupSize.x);

    uint j = uint(workspace[i / 4][i % 4]) - 1;
    return mix(~0, j, hit && j < gl_WorkGroupSize.x);
}

float reduce_min(uint i, bool hit, float value){
    barrier();
    workspace[i / 4][i % 4] = mix(pc.render_distance, value, hit);
    barrier();
    if ((i & 0x01) == 0) workspace[i] = min(workspace[i], workspace[i +   1]);
    if ((i & 0x03) == 0) workspace[i] = min(workspace[i], workspace[i +   2]);
    if ((i & 0x07) == 0) workspace[i] = min(workspace[i], workspace[i +   4]);
    if ((i & 0x0F) == 0) workspace[i] = min(workspace[i], workspace[i +   8]);
    if ((i & 0x1F) == 0) workspace[i] = min(workspace[i], workspace[i +  16]);
    if ((i & 0x3F) == 0) workspace[i] = min(workspace[i], workspace[i +  32]);
    if ((i & 0x7F) == 0) workspace[i] = min(workspace[i], workspace[i +  64]);
    
    vec4 m = workspace[0];
    return min(min(m.x, m.y), min(m.z, m.w));
}

float phi_s_initial(vec3 d, vec3 centre, float r){
    // bug here with -ve values
    float a = dot(d, d);
    float b = -2.0 * dot(centre, d);
    float c = dot(centre, centre) - 3 * r * r;
    float discriminant = b * b - 4 * a * c;
    float dist = (-b - sqrt(discriminant)) / (2.0 * a);
    float result = mix(dist, pc.render_distance, discriminant < 0);
    return max(0, result);
}

request_t render(uint i, substance_t s, vec3 d, float phi_initial){
    request_t request;
    request.status = 0;

    ray_t r = ray_t(pc.camera_position + d * phi_initial, d);
    intersection_t intersection = raycast(r, request);

    reduce_surface_aabb(i, intersection.hit, intersection.x);
    lights_min = lights[0].x;
    lights_max = lights[0].x;

    barrier();

    bool substance_shadow = is_substance_shadow(s);
    uint total;
    uint j = reduce_to_fit(i, substance_shadow, shadow_substances_visible);
    if (j != ~0){
        shadow_substances[j] = s;
    }

    vec4 hit_colour = colour(intersection.texture_coord) * light(lights[0], intersection.x, intersection.normal, intersection.texture_coord, request);
    imageStore(render_texture, ivec2(gl_GlobalInvocationID.xy), mix(sky(), hit_colour, intersection.hit));

    return request;
}

bool is_directly_visible(substance_t sub){
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

float prerender(uint i, uint work_group_id, vec3 d, substance_t s){
    // clear shared variables
    if (i == 0){
        vacant_node = 0;
        chosen_request = ~0;
        lights[0] = light_t(vec3(-3, 3, -3), 0, vec4(50));
        lights_visible = 1;
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
    bool directly_visible = s.id != ~0 && is_directly_visible(s);
    uint j = reduce_to_fit(i, directly_visible, substances_visible);
    if (j != ~0){
        substances[j] = s;
    }
 
    // calculate initial distance
    float phi_initial = reduce_min(i, s.id != ~0 && directly_visible, phi_s_initial(d, s.c, s.r));
    return phi_initial;
}

void postrender(uint i, request_t request){
    barrier();
    bool hit = request.status != 0 && vacant_node != 0;
    barrier();
    uint m = uint(reduce_min(i, hit, i));
    barrier();
    if (i == m){
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

    substance_data_t s_d = substance.data[i];
    vec4 q = vec4(s_d.q & 0xFF, (s_d.q >> 8) & 0xFF, (s_d.q >> 16) & 0xFF, s_d.q >> 24) - 127.5;
    q = normalize(q);
    substance_t s = substance_t(
        s_d.c, s_d.root, s_d.r, s_d.id, get_mat(q), get_mat(vec4(q.x, -q.yzw))
    );

    vec2 uv = uv();
    vec3 up = pc.eye_up;
    vec3 right = pc.eye_right;
    vec3 forward = cross(right, up);
    vec3 d = normalize(forward * pc.focal_depth + right * uv.x + up * uv.y);

    float phi_initial = prerender(i, work_group_id, d, s);

    barrier();
    request_t request = render(i, s, d, phi_initial);

    barrier();

    postrender(i, request);
}
