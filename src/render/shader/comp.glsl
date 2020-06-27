#version 450

layout (local_size_x = 32, local_size_y = 32) in;

layout (binding = 10) uniform writeonly image2D render_texture;
layout (binding = 11) uniform sampler3D normal_texture;
layout (binding = 12) uniform sampler3D colour_texture;

// constants
const uint node_empty_flag = 1 << 24;
const uint node_unused_flag = 1 << 25;
const uint node_child_mask = 0xFFFF;
const int octree_pool_size = int(gl_WorkGroupSize.x * gl_WorkGroupSize.y);

const float sqrt3 = 1.73205080757;
const int max_steps = 64;
const float epsilon = 1.0 / 256.0;

const ivec3 p = ivec3(
    393241,
    1572869,
    98317
);

const uvec4 vertex_masks[2] = {
    uvec4(1 << 16, 1 << 17, 1 << 18, 1 << 19),
    uvec4(1 << 20, 1 << 21, 1 << 22, 1 << 23)
};

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
    int _1;

    vec3 r;
    uint id;

    mat4 transform;
};

struct intersection_t {
    bool hit;
    vec3 x;
    vec3 normal;
    float distance;
    substance_t substance;
    vec3 local_x;
    vec3 cell_position;
    float cell_radius;
    uint index;
};

struct request_t {
    vec3 position;
    float radius;

    uint index;
    uint hash;
    uint substanceID;
    uint status;
};

struct light_t {
    vec3 x;
    uint id;

    vec4 colour;
};

layout (binding = 1) buffer octree_buffer    { uint        data[]; } octree_global;
layout (binding = 2) buffer request_buffer   { request_t   data[]; } requests;
layout (binding = 3) buffer lights_buffer    { light_t     data[]; } lights_global;
layout (binding = 4) buffer substance_buffer { substance_t data[]; } substance;

// shared memory
shared substance_t substances[gl_WorkGroupSize.x];
shared uint substances_visible;

shared substance_t shadows[gl_WorkGroupSize.x];
shared uint shadows_visible;

shared light_t lights[gl_WorkGroupSize.x];
shared uint lights_visible;

shared uint octree[octree_pool_size];

shared bool hitmap[octree_pool_size];
shared vec4 workspace[gl_WorkGroupSize.x * gl_WorkGroupSize.y];

vec2 uv(vec2 xy){
    vec2 uv = xy / (gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    uv = uv * 2.0 - 1.0;
    uv.y *= -float(gl_NumWorkGroups.y) / gl_NumWorkGroups.x;
    return uv;
}

uint expected_order(vec3 x){
    return uint(
        4
        // length((x - pc.camera_position) / 10) +
        // length(uv(gl_GlobalInvocationID.xy))
    );
}

uint work_group_offset(){
    return (gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * octree_pool_size;
}

bool is_leaf(uint i){
    return (octree[i] & node_child_mask) >= octree_pool_size;
}

float phi_s(vec3 global_x, substance_t sub, inout intersection_t intersection, inout request_t request){
    vec3 local_x = (sub.transform * vec4(global_x, 1)).xyz;

    // check against outside bounds of aabb
    float phi_aabb = length(max(abs(local_x) - sub.r, 0)) + epsilon;

    // find the expected size and order of magnitude of cell
    uint order = expected_order(local_x); 
    float radius = epsilon * (1 << order);

    // snap to grid, making sure not to duplicate zero
    vec3 x_scaled = local_x / (radius * 2);
    ivec3 x_grid = ivec3(floor(x_scaled));

    // do a shitty hash to all the relevant fields
    uvec2 os_hash = (ivec2(order, sub.id) * p.x + p.y) % p.z;
    uvec3 x_hash  = (x_grid * p.y + p.z) % p.x;

    uint hash = os_hash.x ^ os_hash.y ^ x_hash.x ^ (x_hash.y * p.x)  ^ (x_hash.z * p.y);

    // calculate some useful variables for doing lookups
    uint index = hash % octree_pool_size;
    hash = hash >> 16;

    vec3 cell_position = x_grid * radius * 2;
    bool is_valid = (octree[index] & 0xFFFF) == hash;

    // if necessary, request more data from CPU
    intersection.local_x = local_x;
    intersection.substance = sub;
    intersection.cell_position = cell_position;
    intersection.index = index;
    intersection.cell_radius = radius;

    if (is_valid){
        hitmap[index] = true;
    } else if (phi_aabb <= epsilon) {
        request = request_t(cell_position, radius, index + work_group_offset(), hash, sub.id, 1);
    }

    uint data = octree[index];
    vec3 alpha = x_scaled - x_grid;

    vec4 phi = mix(sign(data & vertex_masks[0]), sign(data & vertex_masks[1]), alpha.z);
    phi.xy = mix(phi.xy, phi.zw, alpha.y);
    phi.x = mix(phi.x, phi.y, alpha.x);

    phi.x = 2 * radius * mix(1, phi.x - 0.5, is_valid);

    return mix(phi.x, phi_aabb, phi_aabb > epsilon);
}

intersection_t raycast(ray_t r, inout request_t request){
    uint steps;
    intersection_t i;
    
    i.hit = false;
    i.distance = 0;

    for (steps = 0; !i.hit && steps < max_steps; steps++){
        float phi = pc.render_distance;
        for (uint substanceID = 0; !i.hit && substanceID < substances_visible; substanceID++){
            phi = min(phi, phi_s(r.x, substances[substanceID], i, request));
            i.hit = i.hit || phi < epsilon;
        }
        r.x += r.d * phi;
        i.distance += phi;
    }
    
    i.x = r.x;
    return i;
}

float shadow(vec3 l, intersection_t geometry_i, inout request_t request){
    bool hit = false;
    uint steps;

    intersection_t shadow_i;
    ray_t r = ray_t(l, normalize(geometry_i.x - l));
    
    for (steps = 0; !hit && steps < max_steps; steps++){
        float phi = pc.render_distance;
        for (uint substanceID = 0; !hit && substanceID < shadows_visible; substanceID++){
            substance_t sub = shadows[substanceID];
            phi = min(phi, phi_s(r.x, sub, shadow_i, request));
            hit = hit || phi < epsilon;
        }
        r.x += r.d * phi;
    }
    
    bool clear = shadow_i.substance.id == geometry_i.substance.id;
    return float(clear);
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

vec3 get_ray_direction(uvec2 xy){
    vec2 uv = uv(xy);
    vec3 up = pc.eye_up;
    vec3 right = pc.eye_right;
    vec3 forward = cross(right, up);
    return normalize(forward * pc.focal_depth + right * uv.x + up * uv.y);
}

request_t render(uint i, float phi_initial){
    request_t request;
    request.status = 0;

    vec3 d = get_ray_direction(gl_GlobalInvocationID.xy);

    ray_t r = ray_t(pc.camera_position + d * phi_initial, d);
    intersection_t intersection = raycast(r, request);

    const vec4 sky = vec4(0.5, 0.7, 0.9, 1.0);
   
    vec3 t = intersection.local_x - intersection.cell_position + intersection.cell_radius;
    t /= intersection.cell_radius * 4;
    t.xy += vec2(
        (intersection.index + work_group_offset()) % octree_pool_size,
        (intersection.index + work_group_offset()) / octree_pool_size
    );
    t.xy /= vec2(octree_pool_size, gl_NumWorkGroups.x * gl_NumWorkGroups.y);
    
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
    return true;
}

bool is_sphere_visible(vec3 centre, float radius){
    vec3 x = centre - pc.camera_position;
    vec2 image_x = project(x);
    float d = max(epsilon, dot(x, cross(pc.eye_right, pc.eye_up)));
    float r = radius / d * pc.focal_depth * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
    vec2 c = gl_WorkGroupID.xy * gl_WorkGroupSize.xy + gl_WorkGroupSize.xy / 2;
    vec2 diff = max(ivec2(0), abs(c - image_x) - ivec2(gl_WorkGroupSize.xy / 2));

    return length(diff) < r;
}

float prerender(uint i){
    // clear shared variables
    hitmap[i] = false;

    // load shit
    substance_t s = substance.data[i];
    bool directly_visible = s.id != ~0 && is_sphere_visible(s.c, length(s.r));

    light_t l = lights_global.data[i];
    bool light_visible = l.id != ~0;// && is_sphere_visible(l.x, sqrt(length(l.colour) / epsilon));

    // load octree from global memory into shared memory
    octree[i] = octree_global.data[i  + work_group_offset()];
   
    // visibility check on substances and load into shared memory
    barrier();
    bvec4 hits = bvec4(directly_visible, light_visible, false, false);
    uvec4 totals;
    uvec4 limits = uvec4(gl_WorkGroupSize.xx, 0, 0);
    uvec4 indices = reduce_to_fit(i, hits, totals, limits);

    substances_visible = totals.x;
    if (indices.x != ~0){
        substances[indices.x] = s;
    }

    lights_visible = totals.y;
    if (indices.y != ~0){
        lights[indices.y] = l;
    }

    barrier();
    bool shadow_visible = s.id != ~0 && is_shadow_visible(i, vec3(0));
    barrier();
    hits = bvec4(shadow_visible, false, false, false);
    limits = uvec4(gl_WorkGroupSize.x, 0, 0, 0);
    indices = reduce_to_fit(i, hits, totals, limits);
    shadows_visible = totals.x;
    if (indices.x != ~0){
        shadows[indices.x] = s;
    }

    // calculate initial distance
    // float value = mix(pc.render_distance, phi_s_initial(d, s.c, s.r), s.id != ~0 && directly_visible);
    // float phi_initial = reduce_min(i, vec4(value)).x;

    // return phi_initial;
    return 0;
}

void postrender(uint i, request_t request){
    bvec4 hits = bvec4(request.status != 0 && !hitmap[request.index], false, false, false);
    uvec4 _;
    uvec4 limits = uvec4(4, 0, 0, 0);
    uvec4 indices = reduce_to_fit(i, hits, _, limits);
    if (indices.x != ~0){
        requests.data[(gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * 4 + indices.x] = request;
    }
}

void main(){
    uint i = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
    
    float phi_initial = prerender(i);

    barrier();
    request_t request = render(i, phi_initial);
    barrier();

    postrender(i, request);
}
