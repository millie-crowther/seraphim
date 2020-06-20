#version 450

layout (local_size_x = 32, local_size_y = 32) in;

layout (binding = 10) uniform writeonly image2D render_texture;
layout (binding = 11) uniform sampler3D normal_texture;
layout (binding = 12) uniform sampler3D colour_texture;

// constants
const uint node_empty_flag = 1 << 24;
const uint node_unused_flag = 1 << 25;
const uint node_child_mask = 0xFFFF;
const uint octree_pool_size = gl_WorkGroupSize.x * gl_WorkGroupSize.y * 2;

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

struct substance_t {
    vec3 c;
    int root;

    float r;
    float _1;
    float _2;
    uint id;

    mat4 transform;
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

    vec3 node_centre;
    float node_size;
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
layout (binding = 4) buffer substance_buffer { substance_t      data[]; } substance;

// shared memory
shared uvec4 vacant_node;

shared substance_t substances[gl_WorkGroupSize.x];
shared uint substances_visible;

shared substance_t shadows[gl_WorkGroupSize.x];
shared uint shadows_visible;

shared light_t lights[gl_WorkGroupSize.x];
shared uint lights_visible;

shared uint octree[octree_pool_size];

shared bool hitmap[gl_WorkGroupSize.x * gl_WorkGroupSize.y / 8];
shared vec4 workspace[gl_WorkGroupSize.x * gl_WorkGroupSize.y];

vec2 uv(){
    vec2 uv = vec2(gl_GlobalInvocationID.xy) / (gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    uv = uv * 2.0 - 1.0;
    uv.y *= -float(gl_NumWorkGroups.y) / gl_NumWorkGroups.x;
    return uv;
}

float expected_size(vec3 x){
    return 0.05 * (1 + length((x - pc.camera_position) / 10));// + length(uv() / 2));
}

uint work_group_offset(){
    return (gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * octree_pool_size;
}

bool is_leaf(uint i){
    return (octree[i] & node_child_mask) >= octree_pool_size;
}

float phi_s(vec3 _x, substance_t sub, float expected_size, inout intersection_t intersection, inout request_t request){
    vec4 x = sub.transform * vec4(_x, 1);

    // check against outside bounds of aabb
    bool outside_aabb = any(greaterThan(abs(x.xyz), vec3(sub.r + epsilon)));
    float phi_aabb = length(max(abs(x.xyz) - sub.r, 0));

    uint i = sub.root + uint(dot(step(0, x), vec4(1, 2, 4, 0)));
    uint i_prev = i;
    uint depth = 1;

    vec3  c = vec3(0);
    vec3 c_prev = c;
    float s = sub.r;
    vec3 d = step(c, x.xyz);
    c += (d - 0.5) * s;
    s /= 2;

    // perform octree lookup for relevant node
    if (phi_aabb <= epsilon){
        for (; !is_leaf(i); depth++){
            i_prev = i;
            c_prev = c;
            vec3 d = step(c, x.xyz);
            c += (d - 0.5) * s;
            s /= 2;
            i = (octree[i] & node_child_mask) | uint(dot(d, vec3(1, 2, 4)));
            hitmap[i / 8] = true; // TODO: move outside loop
        }
    }

    // hitmap[i_prev / 8] = true;
    // hitmap[i / 8] = true;

    // if necessary, request more data from CPU
    intersection.local_x = x.xyz;
    intersection.parent_index = i_prev;
    intersection.index = i;
    intersection.substance = sub;

    intersection.node_centre = c_prev;
    float node_size = sub.r / (1 << depth);
    intersection.node_size = node_size;

    uint node = octree[i];
    bool node_is_empty = (node & node_empty_flag) != 0;
    bool should_request = node_size >= expected_size && (node & node_child_mask) == node_child_mask && !node_is_empty;
    if (should_request) request = request_t(c, node_size, 0, i, sub.id, 1);
 
    // calculate distance to intersect plane
    float phi_plane = mix(0, node_size * 2, node_is_empty);
    return mix(phi_plane, phi_aabb, outside_aabb);
}

intersection_t raycast(ray_t r, inout request_t request){
    uint steps;
    intersection_t i;
    
    i.hit = false;
    i.distance = 0;

    for (steps = 0; !i.hit && steps < max_steps; steps++){
        float expected_size = expected_size(r.x);
        float phi = pc.render_distance;
        for (uint substanceID = 0; !i.hit && substanceID < substances_visible; substanceID++){
            phi = min(phi, phi_s(r.x, substances[substanceID], expected_size, i, request));
            i.hit = i.hit || phi < epsilon;
        }
        r.x += r.d * phi;
        i.distance += phi;
    }
    
    i.x = r.x;
    return i;
}

float shadow(vec3 l, intersection_t i, inout request_t request){
    bool hit = false;
    uint steps;

    intersection_t _;
    uint hit_sub_id = 0;

    ray_t r = ray_t(l, normalize(i.x - l));
    
    for (steps = 0; !hit && steps < max_steps; steps++){
        float expected_size = expected_size(r.x);
        float phi = pc.render_distance;
        for (uint substanceID = 0; !hit && substanceID < shadows_visible; substanceID++){
            substance_t sub = shadows[substanceID];
            hit_sub_id = sub.id;
            phi = min(phi, phi_s(r.x, sub, expected_size, _, request));
            hit = hit || phi < epsilon;
        }
        r.x += r.d * phi;
    }
    
    bool shadow = hit_sub_id != i.substance.id;
    return float(!shadow);
}

vec4 light(light_t light, intersection_t i, vec3 n, inout request_t request){
    const float shininess = 16;

    // attenuation
    vec3 dist = light.x - i.x;
    float attenuation = 1.0 / dot(dist, dist);

    //shadows
    float shadow = 1;//shadow(light.x, i, request);

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

uvec4 reduce_to_fit(uint i, bvec4 hits, out uvec4 totals, uvec4 limits){
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

    totals = min(uvec4(workspace[1023]), limits);
    barrier();

    bvec4 mask = lessThanEqual(workspace[i], limits) && hits;
    barrier();

    uvec4 result = uvec4(workspace[i]);
    barrier();

    return mix(uvec4(~0), result - 1, mask);
}

vec4 reduce_min(uint i, vec4 value){
    barrier();
    workspace[i] = value;
    barrier();
    if ((i & 0x001) == 0) workspace[i] = min(workspace[i], workspace[i +   1]);
    if ((i & 0x003) == 0) workspace[i] = min(workspace[i], workspace[i +   2]);
    if ((i & 0x007) == 0) workspace[i] = min(workspace[i], workspace[i +   4]);
    if ((i & 0x00F) == 0) workspace[i] = min(workspace[i], workspace[i +   8]);
    if ((i & 0x01F) == 0) workspace[i] = min(workspace[i], workspace[i +  16]);
    if ((i & 0x03F) == 0) workspace[i] = min(workspace[i], workspace[i +  32]);
    if ((i & 0x07F) == 0) workspace[i] = min(workspace[i], workspace[i +  64]);
    if ((i & 0x0FF) == 0) workspace[i] = min(workspace[i], workspace[i + 128]);
    if ((i & 0x1FF) == 0) workspace[i] = min(workspace[i], workspace[i + 256]);
    if ((i & 0x3FF) == 0) workspace[i] = min(workspace[i], workspace[i + 512]);
    
    return workspace[0];
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
   
    vec3 t = intersection.local_x - intersection.node_centre;
    t += intersection.node_size * 4;
    t /= intersection.node_size * 8;
    t.xy += vec2(
        ((intersection.index + work_group_offset()) % octree_pool_size) / 8,
        (intersection.index + work_group_offset()) / octree_pool_size
    );
    t.xy /= vec2(octree_pool_size / 8, gl_NumWorkGroups.x * gl_NumWorkGroups.y);
    
    vec3 n = (
        inverse(intersection.substance.transform) * normalize(vec4(texture(normal_texture, t).xyz - 0.5, 0))
    ).xyz;

    // ambient
    vec4 l = vec4(0.25, 0.25, 0.25, 1.0);

    for (uint i = 0; i < lights_visible; i++){
        l += light(lights[i], intersection, n, request);
    }

    vec4 hit_colour = vec4(texture(colour_texture, t).xyz, 1.0) * l;
    imageStore(render_texture, ivec2(gl_GlobalInvocationID.xy), mix(sky, hit_colour, intersection.hit));

    return request;
}

vec2 project(vec3 x){
    float d = dot(x, cross(pc.eye_right, pc.eye_up));
    vec2 t = vec2(dot(x, pc.eye_right), dot(x, pc.eye_up)) / d * pc.focal_depth;
    t.y *= -float(gl_NumWorkGroups.x) / gl_NumWorkGroups.y;

    return (t + 1) * gl_NumWorkGroups.xy * gl_WorkGroupSize.xy / 2;
}

bool is_shadow_visible(uint i, vec3 x){
    // vec2 p_x = project(x);

    // vec4 bounds = reduce_min(i, vec4(p_x, -p_x));

    // vec2 s_min = bounds.xy;
    // vec2 s_max = -bounds.zw;

    return true;
}

bool is_sphere_visible(vec3 centre, float radius){
    vec3 x = centre - pc.camera_position;

    ivec2 image_x = ivec2(project(x));
    float d = max(epsilon, dot(x, cross(pc.eye_right, pc.eye_up)));
    float r = radius / d * pc.focal_depth * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
    ivec2 c = ivec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy + gl_WorkGroupSize.xy / 2);
    ivec2 diff = max(ivec2(0), abs(c - image_x) - ivec2(gl_WorkGroupSize.xy / 2));

    return length(diff) < r;
}

float prerender(uint i, uint work_group_id, vec3 d){
    // clear shared variables
    if (i == 0){
        vacant_node = uvec4(~0);
    }
    hitmap[i / 8] = false;

    // load shit
    substance_t s = substance.data[i];
    bool directly_visible = s.id != ~0 && is_sphere_visible(s.c, s.r);

    light_t l = lights_global.data[i];
    bool light_visible = l.id != ~0;// && is_sphere_visible(l.x, sqrt(length(l.colour) / epsilon));

        // load octree from global memory into shared memory
    octree_data_t node = octree_global.data[i + work_group_offset()];
    octree[i] = node.structure;
    bool is_node_vacant = (i & 7) == 0 && (node.structure & node_unused_flag) != 0;
   
    // visibility check on substances and load into shared memory
    barrier();
    bvec4 hits = bvec4(directly_visible, light_visible, is_node_vacant, false);
    uvec4 totals;
    uvec4 limits = uvec4(gl_WorkGroupSize.xx, 4, 0);
    uvec4 indices = reduce_to_fit(i, hits, totals, limits);

    substances_visible = totals.x;
    if (indices.x != ~0){
        substances[indices.x] = s;
    }

    lights_visible = totals.y;
    if (indices.y != ~0){
        lights[indices.y] = l;
    }

    if (indices.z != ~0){
        vacant_node[indices.z] = i;
    }

    barrier();
    bool shadow_visible = s.id != ~0 && is_shadow_visible(i, vec3(0));
    barrier();
    hits = bvec4(shadow_visible, false, false, false);
    indices = reduce_to_fit(i, hits, totals, uvec4(gl_WorkGroupSize.x));
    shadows_visible = totals.x;
    if (indices.x != ~0){
        shadows[indices.x] = s;
    }

    if (s.id != ~0) hitmap[s.root / 8] = true;
 
    // calculate initial distance
    float value = mix(pc.render_distance, phi_s_initial(d, s.c, s.r), s.id != ~0 && directly_visible);
    float phi_initial = reduce_min(i, vec4(value)).x;

    // vec3 n = vec3(
    //     node.surface & 0xFF, (node.surface >> 8) & 0xFF, (node.surface >> 16) & 0xFF
    // ) / 127.5 - 1;
    // workspace[i].xyz = n;
    // workspace[i].w = dot(node.centre, n) - (float(node.surface) / 1235007097.17 - sqrt3) * node.size;

    return phi_initial;
}

void postrender(uint i, request_t request){
    bvec4 hits = bvec4(request.status != 0);
    uvec4 _;
    uvec4 limits = uvec4(1, 0, 0, 0);
    uvec4 indices = reduce_to_fit(i, hits, _, limits);
    if (indices.x != ~0 && vacant_node[indices.x] != ~0){
        octree_global.data[request.parent + work_group_offset()].structure &= ~node_child_mask | vacant_node[indices.x];

        request.child = vacant_node[indices.x] + work_group_offset();
        requests.data[(gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * 4 + indices.x] = request;
    }

    // cull leaf nodes that havent been seen this frame
    uint c  = (octree[i] & node_child_mask);
    if (!is_leaf(i) && is_leaf(c) && !hitmap[c / 8]){
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
