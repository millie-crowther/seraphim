#version 450

struct ray_t {
    vec3 x;
    vec3 d;
};

struct substance_t {
    vec3 _2;
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

    vec3 camera_position;
    float phi_initial;        

    vec3 eye_right;
    float focal_depth;

    vec3 eye_up;
    float dummy4;
} pc;

layout (binding = 1) buffer octree_buffer    { uint        data[]; } octree_global;
layout (binding = 2) buffer request_buffer   { request_t   data[]; } requests;
layout (binding = 3) buffer lights_buffer    { light_t     data[]; } lights_global;
layout (binding = 4) buffer substance_buffer { substance_t data[]; } substance;

shared substance_t substances[gl_WorkGroupSize.x];
shared uint substances_visible;

shared substance_t shadows[gl_WorkGroupSize.x];
shared uint shadows_visible;

shared light_t lights[gl_WorkGroupSize.x];
shared uint lights_visible;

aabb_t light_aabb;
aabb_t surface_aabb;

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
    return 4;
    // return max(3, uint(
    //     length(x - pc.camera_position) * 1.5 +
    //     length(uv(gl_GlobalInvocationID.xy))
    // ));
}

uint work_group_offset(){
    return (gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * octree_pool_size;
}

float phi(ray_t global_r, substance_t sub, inout intersection_t intersection, inout request_t request){
    ray_t r = ray_t(
        (sub.transform * vec4(global_r.x, 1)).xyz,
        (sub.transform * vec4(global_r.d, 0)).xyz
    );

    vec3 faces = -sign(r.d) * sub.radius;
    vec3 phis = (faces - r.x) / r.d;
    float phi_aabb = max(phis.x, max(phis.y, phis.z)) + epsilon;

    // check against outside bounds of aabb
    bool inside_aabb = all(lessThan(abs(r.x), sub.radius));
    phi_aabb = mix(pc.render_distance, phi_aabb, phi_aabb > epsilon); 

    // find the expected size and order of magnitude of cell
    uint order = expected_order(r.x); 
    float radius = epsilon * (1 << order);

    // snap to grid, making sure not to duplicate zero
    vec3 x_scaled = r.x / (radius * 2);
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
    intersection.local_x = r.x;
    intersection.substance = sub;
    intersection.cell_position = cell_position;
    intersection.index = index;
    intersection.cell_radius = radius;

    if (is_valid){
        hitmap[index] = true;
    } else if (inside_aabb) {
        request = request_t(cell_position, radius, index + work_group_offset(), hash, sub.id, 1);
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
        for (uint substanceID = 0; !i.hit && substanceID < substances_visible; substanceID++){
            p = min(p, phi(r, substances[substanceID], i, request));
            i.hit = i.hit || p < epsilon;
        }
        r.x += r.d * p;
        i.distance += p;
    }
    
    i.x = r.x;
    return i;
}

float shadow_cast(vec3 l, intersection_t geometry_i, inout request_t request){
    uint steps;
    ray_t r = ray_t(l, normalize(geometry_i.x - l));

    intersection_t shadow_i;
    shadow_i.substance.id = ~0;
    shadow_i.hit = false;
    shadow_i.distance = 0;

    for (steps = 0; !shadow_i.hit && steps < max_steps && shadow_i.distance < pc.render_distance; steps++){
        float p = pc.render_distance;
        for (uint substanceID = 0; !shadow_i.hit && substanceID < shadows_visible; substanceID++){
            p = min(p, phi(r, shadows[substanceID], shadow_i, request));
            shadow_i.hit = shadow_i.hit || p < epsilon;
        }
        r.x += r.d * p;
        shadow_i.distance += p;
    }

    return float(shadow_i.substance.id == geometry_i.substance.id || shadow_i.substance.id == ~0);
}

vec4 light(light_t light, intersection_t i, vec3 n, inout request_t request){
    const float shininess = 16;

    // attenuation
    vec3 dist = light.x - i.x;
    float attenuation = 1.0 / dot(dist, dist);

    //shadows
    float shadow = shadow_cast(light.x, i, request);

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
    vec4 x;
    barrier();
    workspace[i] = value;
    barrier();

    x = min(workspace[i], workspace[i +   1]);
    if ((i &  1) == 0) workspace[i] = x;

    x = min(workspace[i], workspace[i +   2]);
    if ((i &  3) == 0) workspace[i] = x;

    x = min(workspace[i], workspace[i +   4]);
    if ((i &  7) == 0) workspace[i] = x;

    x = min(workspace[i], workspace[i +   8]);
    if ((i & 15) == 0) workspace[i] = x;

    x = min(workspace[i], workspace[i +  16]);
    if ((i & 31) == 0) workspace[i] = x;

    x = min(workspace[i], workspace[i +  32]);
    if ((i & 63) == 0) workspace[i] = x;

    x = min(workspace[i], workspace[i +  64]);
    if ((i & 127) == 0) workspace[i] = x;

    x = min(workspace[i], workspace[i + 128]);
    if ((i & 255) == 0) workspace[i] = x;

    x = min(workspace[i], workspace[i + 256]);
    if ((i & 511) == 0) workspace[i] = x;
    
    return min(workspace[0], workspace[512]);
}

vec3 get_ray_direction(uvec2 xy){
    vec2 uv = uv(xy);
    vec3 up = pc.eye_up;
    vec3 right = pc.eye_right;
    vec3 forward = cross(right, up);
    return normalize(forward * pc.focal_depth + right * uv.x + up * uv.y);
}

request_t render(uint i, substance_t s){
    request_t request;
    request.status = 0;

    vec3 d = get_ray_direction(gl_GlobalInvocationID.xy);

    ray_t r = ray_t(pc.camera_position, d);
    intersection_t intersection = raycast(r, request);

    vec4 x = intersection.x.xyzz;
    barrier();
    vec4 lower = reduce_min(i, mix(vec4(pc.render_distance),  x, intersection.hit));
    barrier();
    vec4 upper = reduce_min(i, mix(vec4(pc.render_distance), -x, intersection.hit));
    surface_aabb = aabb_t(lower.xyz, -upper.xyz);

    barrier();
    bool shadow_visible = s.id != ~0;
    bvec4 hits = bvec4(shadow_visible, false, false, false);
    uvec4 limits = uvec4(gl_WorkGroupSize.x, 0, 0, 0);
    uvec4 totals;
    barrier();
    uvec4 indices = reduce_to_fit(i, hits, totals, limits);
    barrier();
    shadows_visible = totals.x;
    barrier();
    if (indices.x != ~0){
        shadows[indices.x] = s;
    }
    barrier();
    
    uint j = intersection.index + work_group_offset();
    vec3 t = intersection.local_x - intersection.cell_position + intersection.cell_radius;
    t /= intersection.cell_radius * 4;
    t.xy += vec2(j % octree_pool_size, j / octree_pool_size);
    t.xy /= vec2(octree_pool_size, gl_NumWorkGroups.x * gl_NumWorkGroups.y);
    
    vec3 n = 
        inverse(mat3(intersection.substance.transform)) * 
        normalize(texture(normal_texture, t).xyz - 0.5);

    // ambient
    vec4 l = vec4(0.25, 0.25, 0.25, 1.0);

    for (uint i = 0; i < lights_visible; i++){
        l += light(lights[i], intersection, n, request);
    }

    const vec4 sky = vec4(0.5, 0.7, 0.9, 1.0);
    vec4 hit_colour = vec4(texture(colour_texture, t).xyz, 1.0) * l;
    vec4 image_colour = mix(sky, hit_colour, intersection.hit);
    imageStore(render_texture, ivec2(gl_GlobalInvocationID.xy), image_colour);

    return request;
}

vec2 project(vec3 x, mat4 transform){
    x = (inverse(transform) * vec4(x, 1)).xyz;
    x -= pc.camera_position;
    float d = dot(x, cross(pc.eye_right, pc.eye_up));
    d = max(epsilon, d);
    vec2 t = vec2(dot(x, pc.eye_right), dot(x, pc.eye_up)) / d * pc.focal_depth;
    t.y *= -float(gl_NumWorkGroups.x) / gl_NumWorkGroups.y;

    return (t + 1) * gl_NumWorkGroups.xy * gl_WorkGroupSize.xy / 2;
}

bool plane_intersect_aabb(vec3 x, vec3 n, aabb_t aabb){
    // TODO: check if this still handles intersections that are behind the camera!
    float d = dot(x, n);

    bvec3 is_not_null = greaterThanEqual(abs(n), vec3(epsilon));

    vec2 ax = n.x * vec2(aabb.lower.x, aabb.upper.x);
    vec2 by = n.y * vec2(aabb.lower.y, aabb.upper.y);
    vec2 cz = n.z * vec2(aabb.lower.z, aabb.upper.z);

    // ax + by + cz = d
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

    return any(!(lt || gt) && is_not_null);
}

bool is_substance_visible(substance_t sub){
    vec2 lower = gl_WorkGroupID.xy * gl_WorkGroupSize.xy;
    vec2 upper = (gl_WorkGroupID.xy + 1) * gl_WorkGroupSize.xy;

    // check if centre of substance projects into work group
    vec2 c = project(vec3(0), sub.transform);
    bvec2 c_hit = greaterThanEqual(c, lower) && lessThan(c, upper);

    vec3 o = (sub.transform * vec4(pc.camera_position, 1)).xyz;

    mat4x3 rays = mat3(sub.transform) * mat4x3(
        get_ray_direction( gl_WorkGroupID.xy                * gl_WorkGroupSize.xy),
        get_ray_direction((gl_WorkGroupID.xy + uvec2(0, 1)) * gl_WorkGroupSize.xy),
        get_ray_direction((gl_WorkGroupID.xy + uvec2(1, 0)) * gl_WorkGroupSize.xy),
        get_ray_direction((gl_WorkGroupID.xy + uvec2(1, 1)) * gl_WorkGroupSize.xy)
    );

    bvec4 rays_hit = bvec4(
        plane_intersect_aabb(o, cross(rays[0], rays[1]), aabb_t(-sub.radius, sub.radius)),
        plane_intersect_aabb(o, cross(rays[1], rays[2]), aabb_t(-sub.radius, sub.radius)),
        plane_intersect_aabb(o, cross(rays[2], rays[3]), aabb_t(-sub.radius, sub.radius)),
        plane_intersect_aabb(o, cross(rays[3], rays[0]), aabb_t(-sub.radius, sub.radius))
    );

    return (all(c_hit) || any(rays_hit)) && sub.id != ~0;
}

void prerender(uint i, substance_t s){
    // clear shared variables
    hitmap[i] = false;

    // load shit
    bool directly_visible = is_substance_visible(s);

    light_t l = lights_global.data[i];
    bool light_visible = l.id != ~0;

    // load octree from global memory into shared memory
    octree[i] = octree_global.data[i + work_group_offset()];
   
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
    
    substance_t s = substance.data[i];
    prerender(i, s);

    barrier();
    request_t request = render(i, s);
    barrier();

    postrender(i, request);
}
