#version 450

struct ray_t {
    vec3 x;
    vec3 d;
};

struct substance_t {
    vec3 c;
    int _1;

    vec3 radius;
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
    uint global_index;
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

    vec3 colour;
    uint index;
};

struct aabb_t {
    vec3 lower;
    vec3 upper;
};

layout (local_size_x = 32, local_size_y = 32) in;

const uint node_empty_flag = 1 << 24;
const uint node_unused_flag = 1 << 25;
const uint node_child_mask = 0xFFFF;
const int octree_pool_size = int(gl_WorkGroupSize.x * gl_WorkGroupSize.y);

const float sqrt3 = 1.73205080757;
const int max_steps = 32;
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

layout (binding = 10) uniform writeonly image2D render_texture;
layout (binding = 11) uniform sampler3D normal_texture;
layout (binding = 12) uniform sampler3D colour_texture;

layout( push_constant ) uniform push_constants {
    uvec2 window_size;
    float render_distance;
    uint current_frame;

    float phi_initial;        
    float focal_depth;
    uint number_of_calls;
    float _1;

    mat4 eye_transform;
} pc;

layout (binding = 1) buffer octree_buffer    { uint        data[]; } octree_global;
layout (binding = 2) buffer request_buffer   { request_t   data[]; } requests;
layout (binding = 3) buffer lights_buffer    { light_t     data[]; } lights_global;
layout (binding = 4) buffer substance_buffer { substance_t data[]; } substance;
layout (binding = 5) buffer pointer_buffer   { uint        data[]; } pointers;
layout (binding = 6) buffer frustum_buffer   { vec2        data[]; } frustum;

shared substance_t substances[gl_WorkGroupSize.x];
shared uint substances_size;

shared light_t lights[gl_WorkGroupSize.x];
shared uint lights_size;

shared uint octree[octree_pool_size];

shared vec4 workspace[gl_WorkGroupSize.x * gl_WorkGroupSize.y];

int patch_pool_size(){
    return int(gl_NumWorkGroups.x * gl_NumWorkGroups.y * gl_WorkGroupSize.x * gl_WorkGroupSize.y);
}

int number_of_lights(){
    return int(min(gl_WorkGroupSize.x * gl_WorkGroupSize.y, gl_NumWorkGroups.x * gl_NumWorkGroups.y));
}

int number_of_substances(){
    return int(gl_WorkGroupSize.x * gl_WorkGroupSize.y);
}

vec2 uv(vec2 xy){
    vec2 uv = xy / (gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    uv = uv * 2.0 - 1.0;
    uv.y *= -float(gl_NumWorkGroups.y) / gl_NumWorkGroups.x;
    return uv;
}

uint expected_order(vec3 x){
    return 15;
}

uint work_group_offset(){
    return (gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * octree_pool_size;
}

float phi(ray_t global_r, substance_t sub, inout intersection_t intersection, inout request_t request){
    ray_t r = ray_t(
        (sub.transform * vec4(global_r.x, 1)).xyz,
        mat3(sub.transform) * global_r.d
    );

    vec3 faces = -sign(r.d) * sub.radius;
    vec3 phis = (faces - r.x) / r.d;
    float phi_aabb = max(phis.x, max(phis.y, phis.z));

    // TODO: feel like you should be able to use length(max(phis, 0)) here 
    //       instead of max(phis.x, phis.y, phis.z)

    // check against outside bounds of aabb
    bool inside_aabb = all(lessThan(abs(r.x), sub.radius));
    phi_aabb = mix(pc.render_distance, phi_aabb, phi_aabb > 0) + epsilon; 

    // find the expected size and order of magnitude of cell
    uint order = expected_order(r.x); 
    float radius = epsilon * order;

    // snap to grid, making sure not to duplicate zero
    vec3 x_scaled = r.x / (radius * 2);
    ivec3 x_grid = ivec3(floor(x_scaled));

    // do a shitty hash to all the relevant fields
    uvec2 os_hash = (ivec2(order, sub.id) * p.x + p.y) % p.z;
    uvec3 x_hash  = (x_grid * p.y + p.z) % p.x;

    uint hash = os_hash.x ^ os_hash.y ^ x_hash.x ^ (x_hash.y * p.x)  ^ (x_hash.z * p.y);

    // calculate some useful variables for doing lookups
    uint index = hash % octree_pool_size;

    vec3 cell_position = x_grid * radius * 2;
    bool is_valid = (octree[index] & 0xFFFF) == hash >> 16;

    // if necessary, request more data from CPU
    intersection.local_x = r.x;
    intersection.substance = sub;
    intersection.cell_position = cell_position;
    intersection.index = index;
    intersection.cell_radius = radius;
    intersection.global_index = hash % patch_pool_size();

    if (inside_aabb && !is_valid) {
        request = request_t(cell_position, radius, hash % patch_pool_size(), hash, sub.id, 1);
    }

    uint data = octree[index];
    vec3 alpha = x_scaled - x_grid;

    vec4 phi = mix(sign(data & vertex_masks[0]), sign(data & vertex_masks[1]), alpha.z);
    phi.xy = mix(phi.xy, phi.zw, alpha.y);
    phi.x = mix(phi.x, phi.y, alpha.x);

    phi.x = 2 * radius * mix(1, phi.x - 0.5, is_valid);

    return mix(phi_aabb, phi.x, inside_aabb);
}

intersection_t raycast(ray_t r, inout request_t request){
    uint steps;
    intersection_t i;
    
    i.hit = false;
    i.distance = 0;

    for (steps = 0; !i.hit && steps < max_steps && i.distance < pc.render_distance; steps++){
        float p = pc.render_distance;
        for (uint substanceID = 0; !i.hit && substanceID < substances_size; substanceID++){
            p = min(p, phi(r, substances[substanceID], i, request));
            i.hit = i.hit || p < epsilon;
        }
        r.x += r.d * p;
        i.distance += p;
    }
    
    i.x = r.x;
    return i;
}

float shadow_cast(vec3 l, uint light_i, intersection_t geometry_i, inout request_t request){
    uint steps;
    ray_t r = ray_t(l, normalize(geometry_i.x - l));

    intersection_t shadow_i;
    shadow_i.substance.id = ~0;
    shadow_i.hit = false;  
    shadow_i.distance = 0;

    for (steps = 0; !shadow_i.hit && steps < max_steps && shadow_i.distance < pc.render_distance; steps++){
        float p = pc.render_distance;

        for (uint i = 0; !shadow_i.hit && i < substances_size; i++){
            p = min(p, phi(r, substances[i], shadow_i, request));
            shadow_i.hit = shadow_i.hit || p < epsilon;
        }
        r.x += r.d * p;
        shadow_i.distance += p;
    }

    return float(shadow_i.substance.id == geometry_i.substance.id || shadow_i.substance.id == ~0);
}

vec4 light(uint light_i, intersection_t i, vec3 n, inout request_t request){
    const float shininess = 16;

    light_t light = lights[light_i];

    // attenuation
    vec3 dist = light.x - i.x;
    float attenuation = 1.0 / dot(dist, dist);

    //shadows
    float shadow = shadow_cast(light.x, light_i, i, request);

    //diffuse
    vec3 l = normalize(light.x - i.x);
    float d = 0.75 * max(epsilon, dot(l, n));

    //specular
    vec3 v = normalize(-i.x);

    vec3 h = normalize(l + v);
    float s = 0.4 * pow(max(dot(h, n), 0.0), shininess);

    return (d + s) * attenuation * shadow * vec4(light.colour, 1);
}

uvec4 reduce_to_fit(uint i, bvec4 hits, out uvec4 totals, uvec4 limits){
    vec4 x;
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
    workspace[i] = value;

    if ((i &   1) == 0) workspace[i] = min(workspace[i], workspace[i +   1]);
    if ((i &   3) == 0) workspace[i] = min(workspace[i], workspace[i +   2]);
    if ((i &   7) == 0) workspace[i] = min(workspace[i], workspace[i +   4]);
    if ((i &  15) == 0) workspace[i] = min(workspace[i], workspace[i +   8]);
    if ((i &  31) == 0) workspace[i] = min(workspace[i], workspace[i +  16]);
    if ((i &  63) == 0) workspace[i] = min(workspace[i], workspace[i +  32]);
    if ((i & 127) == 0) workspace[i] = min(workspace[i], workspace[i +  64]);
    if ((i & 255) == 0) workspace[i] = min(workspace[i], workspace[i + 128]);
    if ((i & 511) == 0) workspace[i] = min(workspace[i], workspace[i + 256]);
    
    return min(workspace[0], workspace[512]);
}

vec3 get_ray_direction(uvec2 xy){
    vec2 uv = uv(xy);
    return normalize(mat3(pc.eye_transform) * vec3(uv.x, uv.y, pc.focal_depth));
}

bool is_light_visible(light_t l, float near, float far){
    return l.id != ~0 && near < far;
}

bool is_shadow_visible(substance_t s, float near, float far){
    return s.id != ~0 && near < far;
}

request_t render(uint i, uint j, substance_t s, uint shadow_index, uint shadow_size){
    request_t request;
    request.status = 0;

    vec3 rx = inverse(pc.eye_transform)[3].xyz;
    vec3 d = get_ray_direction(gl_GlobalInvocationID.xy);

    ray_t r = ray_t(rx, d);
    intersection_t intersection = raycast(r, request);

    barrier();

    // find near / far planes of intersection points
    vec2 x = vec2(intersection.distance, -intersection.distance);
    vec4 result = reduce_min(i, mix(vec4(pc.render_distance), x.xyxy, intersection.hit));
    
    barrier();
    
    frustum.data[j] = vec2(result.x, -result.y);

    barrier();

    substances_size = shadow_size;
    if (shadow_index != ~0){
        substances[shadow_index] = s;
    }
    
    // find texture coordinate
    uint k = intersection.global_index;
    vec3 t = intersection.local_x - intersection.cell_position + intersection.cell_radius;
    t /= intersection.cell_radius * 4;
    t.xy += vec2(k % octree_pool_size, k / octree_pool_size);
    t.xy /= vec2(octree_pool_size, gl_NumWorkGroups.x * gl_NumWorkGroups.y);
    
    vec3 n = 
        inverse(mat3(intersection.substance.transform)) * 
        normalize(texture(normal_texture, t).xyz - 0.5);

    // ambient
    vec4 lighting = vec4(0.25, 0.25, 0.25, 1.0);

    for (uint light_i = 0; light_i < lights_size; light_i++){
        lighting += light(light_i, intersection, n, request);
    }

    const vec4 sky = vec4(0.5, 0.7, 0.9, 1.0);
    vec4 hit_colour = vec4(texture(colour_texture, t).xyz, 1.0) * lighting;
    vec4 image_colour = mix(sky, hit_colour, intersection.hit);
    imageStore(render_texture, ivec2(gl_GlobalInvocationID.xy), image_colour);

    return request;
}

bool plane_intersect_aabb(vec3 x, vec3 n, aabb_t aabb){
    // TODO: check if this still handles intersections that are behind the camera!
    // TODO: improve this

    bvec3 is_not_null = greaterThanEqual(abs(n), vec3(epsilon));
 
    vec2 ax = n.x * vec2(aabb.lower.x, aabb.upper.x);
    vec2 by = n.y * vec2(aabb.lower.y, aabb.upper.y);
    vec2 cz = n.z * vec2(aabb.lower.z, aabb.upper.z);

    // ax + by + cz = d
    float d = dot(x, n);
    vec4 xs = (d - by.xxyy - cz.xyxy) / n.x;
    vec4 ys = (d - ax.xxyy - cz.xyxy) / n.y;
    vec4 zs = (d - ax.xxyy - by.xyxy) / n.z;

    bvec3 lt = bvec3(
        all(lessThan(xs, aabb.lower.xxxx)),
        all(lessThan(ys, aabb.lower.yyyy)),
        all(lessThan(zs, aabb.lower.zzzz))
    );

    bvec3 gt = bvec3(
        all(greaterThan(xs, aabb.upper.xxxx)),
        all(greaterThan(ys, aabb.upper.yyyy)),
        all(greaterThan(zs, aabb.upper.zzzz))
    );

    return any(is_not_null && !(lt || gt));
}

bool is_substance_visible(substance_t sub, mat4x3 rays_global){
    vec2 lower = gl_WorkGroupID.xy * gl_WorkGroupSize.xy;
    vec2 upper = (gl_WorkGroupID.xy + 1) * gl_WorkGroupSize.xy;

    // check if centre of substance projects into work group
    vec2 c = (inverse(pc.eye_transform) * inverse(sub.transform) * vec4(0, 0, 0, 1)).xy;
    bool c_hit = all(greaterThanEqual(c, lower) && lessThan(c, upper));

    vec3 o = (sub.transform * inverse(pc.eye_transform)[3]).xyz;

    mat4x3 rays = mat3(sub.transform) * rays_global;

    bvec4 rays_hit = bvec4(
        plane_intersect_aabb(o, cross(rays[0], rays[1]), aabb_t(-sub.radius, sub.radius)),
        plane_intersect_aabb(o, cross(rays[1], rays[2]), aabb_t(-sub.radius, sub.radius)),
        plane_intersect_aabb(o, cross(rays[2], rays[3]), aabb_t(-sub.radius, sub.radius)),
        plane_intersect_aabb(o, cross(rays[3], rays[0]), aabb_t(-sub.radius, sub.radius))
    );

    return (c_hit || any(rays_hit)) && sub.id != ~0;
}

void prerender(uint i, uint j, substance_t s, out uint shadow_index, out uint shadow_size){
    // clear things
    lights[gl_LocalInvocationID.x].index = ~0;

    mat4x3 rays = mat4x3(
        get_ray_direction( gl_WorkGroupID.xy                * gl_WorkGroupSize.xy),
        get_ray_direction((gl_WorkGroupID.xy + uvec2(0, 1)) * gl_WorkGroupSize.xy),
        get_ray_direction((gl_WorkGroupID.xy + uvec2(1, 0)) * gl_WorkGroupSize.xy),
        get_ray_direction((gl_WorkGroupID.xy + uvec2(1, 1)) * gl_WorkGroupSize.xy)
    );

    // load shit
    bool directly_visible = is_substance_visible(s, rays);
    light_t l = lights_global.data[i];
    vec2 f = frustum.data[j].xy;
    bool light_visible = is_light_visible(l, f.x, f.y) && i < number_of_lights();
    bool shadow_visible = is_shadow_visible(s, f.x, f.y);

    // load octree from global memory into shared memory
    uint patch_index = pointers.data[i + work_group_offset()];
    octree[i] = octree_global.data[patch_index];
   
    // visibility check on substances and load into shared memory
    barrier();
    bvec4 hits = bvec4(directly_visible, light_visible, shadow_visible, false);
    uvec4 limits = uvec4(gl_WorkGroupSize.xxx, 0);
    uvec4 totals;
    uvec4 indices = reduce_to_fit(i, hits, totals, limits);

    substances_size = totals.x;
    if (indices.x != ~0){
        substances[indices.x] = s;
    }

    lights_size = totals.y;
    if (indices.y != ~0){
        lights[indices.y] = l;
    }

    shadow_index = indices.z;
    shadow_size = totals.z;
}

void postrender(uint i, request_t request){
    // TODO: performance test on putting this inside the lookup function
    if (request.status != 0){
        uint index_local = request.hash % octree_pool_size + work_group_offset();
        pointers.data[index_local] = request.index; 

        if ((octree_global.data[request.index] & 0xFFFF) != request.hash >> 16){
            uint index = request.hash % pc.number_of_calls;
            request.hash = request.hash >> 16;
            requests.data[index] = request;
        }
    }
}

void main(){
    uint i = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
    uint j = gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x;
    
    substance_t s = substance.data[i];
    uint shadow_index, shadow_size;
    prerender(i, j, s, shadow_index, shadow_size);

    barrier();
    request_t request = render(i, j, s, shadow_index, shadow_size);
    barrier();

    postrender(i, request);
}
