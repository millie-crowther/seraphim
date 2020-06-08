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

struct ray_t {
    vec3 x;
    vec3 d;
};

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

struct intersection_t {
    bool hit;
    vec3 x;
    vec3 normal;
    float distance;
    uint index;
    uint parent_index;
    substance_t substance;
    vec3 local_x;
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
    uint id;

    vec4 colour;
};

struct octree_data_t {
    uint structure;
    uint surface;
    uint _1;
    uint _2;

    vec3 centre;
    float size;
};

layout (binding = 1) buffer octree_buffer    { octree_data_t    data[]; } octree_global;
layout (binding = 2) buffer request_buffer   { request_t        data[]; } requests;
layout (binding = 3) buffer lights_buffer    { light_t          data[]; } lights_global;
layout (binding = 4) buffer substance_buffer { substance_data_t data[]; } substance;

// shared memory
shared uint vacant_node;
shared uint chosen_request;

shared substance_t substances[gl_WorkGroupSize.x];
shared uint substances_visible;

shared substance_t shadow_substances[gl_WorkGroupSize.x];
shared uint shadow_substances_visible;

shared light_t lights[32];
shared uint lights_visible;

shared uint octree[gl_WorkGroupSize.x * gl_WorkGroupSize.y];
shared vec3 node_centres[gl_WorkGroupSize.x * gl_WorkGroupSize.y];
shared float node_sizes[gl_WorkGroupSize.x * gl_WorkGroupSize.y];

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

float phi_s(ray_t r, substance_t sub, float expected_size, inout intersection_t intersection, inout request_t request){
    r.x -= sub.c;
    r.x = sub.rotation * r.x;

    // check against outside bounds of aabb
    float phi_aabb = length(max(abs(r.x) - sub.r, 0));
    bool outside_aabb = phi_aabb > epsilon;

    uint i = sub.root + uint(dot(step(0, r.x), vec3(1, 2, 4)));
    uint i_prev = i;
    uint next = octree[i].x & node_child_mask;

    // perform octree lookup for relevant node
    while (!outside_aabb && next != node_child_mask && (octree[next] & node_unused_flag) == 0){
        vec3 c = node_centres[i];
        i_prev = i;
        i = next | uint(dot(step(c, r.x), vec3(1, 2, 4)));
        hitmap[i / 8] = true;
        next = octree[i] & node_child_mask;
    }

    // calculate distance to intersect plane
    float phi_plane = min(0, workspace[i].w - dot(r.x, workspace[i].xyz)) / dot(r.d, workspace[i].xyz);

    // if necessary, request more data from CPU
    bool is_not_empty = (octree[i] & node_empty_flag) == 0;
    bool should_request = node_sizes[i] >= expected_size && is_not_empty && next == node_child_mask;
    if (should_request) request = request_t(node_centres[i], node_sizes[i], 0, i, sub.id, 1);

    intersection.local_x = r.x;
    intersection.parent_index = i_prev;
    intersection.index = i;
    intersection.substance = sub;
 
    float phi_interior = mix(node_sizes[i], phi_plane, is_not_empty && phi_plane >= 0);
    return mix(phi_interior, phi_aabb, outside_aabb);
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
            phi = min(phi, phi_s(r, substances[substanceID], expected_size, i, request));
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
    intersection_t _;
    for (uint i = 1; i < n; i++){
        for (uint substanceID = 0; substanceID < shadow_substances_visible; substanceID++){
            phi = min(phi, phi_s(ray_t(l + d * i, rd), shadow_substances[substanceID], expected_size, _, request));
        }
    }

    return float(phi > epsilon);
}

vec4 light(light_t light, intersection_t i, vec3 t, inout request_t request){
    const float shininess = 16;

    vec3 n = i.substance.inverse * normalize(texture(normal_texture, t).xyz - 0.5);
    
    // attenuation
    vec3 dist = light.x - i.x;
    float attenuation = 1.0 / dot(dist, dist);

    //ambient 

    //shadows
    float shadow = 1;//shadow(light.x, i.x, request);

    //diffuse
    vec3 l = normalize(light.x - i.x);
    float d = 0.75 * max(epsilon, dot(l, n));

    //specular
    vec3 v = normalize(-i.x);

    vec3 h = normalize(l + v);
    float s = 0.4 * pow(max(dot(h, n), 0.0), shininess);

    return (d + s) * attenuation * shadow * light.colour;
}

bool is_substance_shadow(substance_t s){
    vec3 c_min = min(lights_min, surface_min);
    vec3 c_max = max(lights_max, surface_max);
    vec3 d = max(vec3(0), abs((c_max + c_min) / 2 - s.c) - (c_max - c_min) / 2);
    return length(d) < length(vec3(s.r));
}

uvec4 reduce_to_fit(uint i, bvec4 hits, out uvec4 totals){
    barrier();
    workspace[i] = uvec4(hits);
    barrier();

    if ((i &   1) != 0) workspace[i] += workspace[i &   ~1      ];    
    barrier();
    if ((i &   2) != 0) workspace[i] += workspace[i &   ~2 |   1];    
    barrier();
    if ((i &   4) != 0) workspace[i] += workspace[i &   ~4 |   3];    
    barrier();
    if ((i &   8) != 0) workspace[i] += workspace[i &   ~8 |   7];    
    barrier();
    if ((i &  16) != 0) workspace[i] += workspace[i &  ~16 |  15];    
    barrier();
    if ((i &  32) != 0) workspace[i] += workspace[i &  ~32 |  31];    
    barrier();
    if ((i &  64) != 0) workspace[i] += workspace[i &  ~64 |  63];    
    barrier();
    if ((i & 128) != 0) workspace[i] += workspace[i & ~128 | 127];    
    barrier();
    if ((i & 256) != 0) workspace[i] += workspace[i & ~256 | 255];    
    barrier();
    if ((i & 512) != 0) workspace[i] += workspace[           511];    
    barrier();

    totals = min(uvec4(workspace[1023]), gl_WorkGroupSize.x);
    barrier();

    bvec4 mask = lessThanEqual(workspace[i], vec4(gl_WorkGroupSize.x)) && hits;
    barrier();

    uvec4 result = uvec4(workspace[i]);
    barrier();

    return mix(uvec4(~0), result - 1, mask);
}

float reduce_min(uint i, bool hit, float value){
    barrier();
    workspace[i / 4][i % 4] = mix(pc.render_distance, value, hit);
    barrier();
    if ((i & 0x01) == 0) workspace[i] = min(workspace[i], workspace[i +  1]);
    if ((i & 0x03) == 0) workspace[i] = min(workspace[i], workspace[i +  2]);
    if ((i & 0x07) == 0) workspace[i] = min(workspace[i], workspace[i +  4]);
    if ((i & 0x0F) == 0) workspace[i] = min(workspace[i], workspace[i +  8]);
    if ((i & 0x1F) == 0) workspace[i] = min(workspace[i], workspace[i + 16]);
    if ((i & 0x3F) == 0) workspace[i] = min(workspace[i], workspace[i + 32]);
    if ((i & 0x7F) == 0) workspace[i] = min(workspace[i], workspace[i + 64]);
    
    vec4 m = workspace[0];
    return min(min(m.x, m.y), min(m.z, m.w));
}

float phi_s_initial(vec3 d, vec3 centre, float r){
    float a = dot(d, d);
    float b = -2.0 * dot(centre, d);
    float c = dot(centre, centre) - 3 * r * r;
    float discriminant = b * b - 4 * a * c;
    float dist = (-b - sqrt(discriminant)) / (2.0 * a);
    float result = mix(dist, pc.render_distance, discriminant < 0);
    return max(0, result);
}

request_t render(uint i, vec3 d, float phi_initial){
    request_t request;
    request.status = 0;

    ray_t r = ray_t(pc.camera_position + d * phi_initial, d);
    intersection_t intersection = raycast(r, request);

    const vec4 sky = vec4(0.5, 0.7, 0.9, 1.0);
   
    vec3 t = intersection.local_x - node_centres[intersection.parent_index]; 
    t += node_sizes[intersection.index] * 4;
    t /= node_sizes[intersection.index] * 8;
    t.xy += vec2(
        (intersection.index + work_group_offset()) % (gl_WorkGroupSize.x * gl_NumWorkGroups.x) / 8,
        (intersection.index + work_group_offset()) / (gl_WorkGroupSize.x * gl_NumWorkGroups.x)
    );
    t.xy /= gl_WorkGroupSize.xy * gl_NumWorkGroups.xy / vec2(8, 1);

    // ambient
    vec4 l = vec4(0.25, 0.25, 0.25, 1.0);

    for (uint i = 0; i < lights_visible; i++){
        l += light(lights[i], intersection, t, request);
    }

    vec4 hit_colour = vec4(texture(colour_texture, t).xyz, 1.0) * l;
    imageStore(render_texture, ivec2(gl_GlobalInvocationID.xy), mix(sky, hit_colour, intersection.hit));

    return request;
}

bool is_sphere_visible(vec3 centre, float radius){
    vec3 x = centre - pc.camera_position;
    float d = dot(x, cross(pc.eye_right, pc.eye_up));
    vec2 t = vec2(dot(x, pc.eye_right), dot(x, pc.eye_up)) / d * pc.focal_depth;
    t.y *= -float(gl_NumWorkGroups.x) / gl_NumWorkGroups.y;

    ivec2 image_x = ivec2((t + 1) * gl_NumWorkGroups.xy * gl_WorkGroupSize.xy) / 2;
    float r = radius / d * pc.focal_depth * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
    ivec2 c = ivec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy + gl_WorkGroupSize.xy / 2);
    ivec2 diff = max(ivec2(0), abs(c - image_x) - ivec2(gl_WorkGroupSize.xy / 2));

    return length(diff) < r;
}

float prerender(uint i, uint work_group_id, vec3 d){
    // clear shared variables
    if (i == 0){
        vacant_node = 0;
        chosen_request = ~0;
    }
    hitmap[i / 8] = false;

    // load shit
    substance_data_t s_d = substance.data[i];
    vec4 q = vec4(s_d.q & 0xFF, (s_d.q >> 8) & 0xFF, (s_d.q >> 16) & 0xFF, s_d.q >> 24) - 127.5;
    q = normalize(q);
    substance_t s = substance_t(
        s_d.c, s_d.root, s_d.r, s_d.id, get_mat(q), get_mat(vec4(q.x, -q.yzw))
    );
    bool directly_visible = s.id != ~0 && is_sphere_visible(s.c, s.r);

    light_t l = lights_global.data[i];
    bool light_visible = l.id != ~0;// && is_sphere_visible(light.x, )

    // visibility check on substances and load into shared memory
    barrier();
    bvec4 hits = bvec4(directly_visible, light_visible, false, false);
    uvec4 totals;
    uvec4 indices = reduce_to_fit(i, hits, totals);

    substances_visible = totals.x;
    if (indices.x != ~0){
        substances[indices.x] = s;
    }

    lights_visible = totals.y;
    if (indices.y != ~0){
        lights[indices.y] = l;
    }

    if (s.id != ~0) hitmap[s.root / 8] = true;
 
    // calculate initial distance
    float phi_initial = reduce_min(i, s.id != ~0 && directly_visible, phi_s_initial(d, s.c, s.r));

    // load octree from global memory into shared memory
    octree_data_t node = octree_global.data[i + work_group_offset()];
    octree[i] = node.structure;
   
    // submit free nodes to request queue
    if ((i & 7) == 0 && (node.structure & node_unused_flag) != 0){
        vacant_node = i; 
    }

    vec3 n = vec3(
        node.surface & 0xFF, (node.surface >> 8) & 0xFF, (node.surface >> 16) & 0xFF
    ) / 127.5 - 1;
    workspace[i].xyz = n;
    workspace[i].w = dot(node.centre, n) - (float(node.surface) / 1235007097.17 - sqrt3) * node.size;

    node_centres[i] = node.centre;
    node_sizes[i] = node.size;

    return phi_initial;
}

void postrender(uint i, request_t request){
    barrier();
    bool hit = request.status != 0 && vacant_node != 0;
    barrier();
    uint m = uint(reduce_min(i, hit, i));
    barrier();
    if (i == m){
        octree_global.data[request.parent + work_group_offset()].structure &= ~node_child_mask | vacant_node;

        request.child = vacant_node + work_group_offset();
        requests.data[uint(dot(gl_WorkGroupID.xy, vec2(1, gl_NumWorkGroups.x)))] = request;
    }

    // cull nodes that havent been seen this frame
    uint c = (octree[i].x & node_child_mask);
    if (c != node_child_mask && !hitmap[c >> 3]){
        octree_global.data[i + work_group_offset()].structure |= node_child_mask;
        octree_global.data[c + work_group_offset()].structure |= node_unused_flag;
    }
}

void main(){
    uint work_group_id = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
    uint i = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;

    vec2 uv = uv();
    vec3 up = pc.eye_up;
    vec3 right = pc.eye_right;
    vec3 forward = cross(right, up);
    vec3 d = normalize(forward * pc.focal_depth + right * uv.x + up * uv.y);

    float phi_initial = prerender(i, work_group_id, d);

    barrier();
    request_t request = render(i, d, phi_initial);

    barrier();

    postrender(i, request);
}
