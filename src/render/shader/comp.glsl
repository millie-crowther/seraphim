#version 450

struct ray_t {
    vec3 x;
    vec3 d;
};

struct substance_t {
    float near;
    float far;
    float _1;
    float _2;

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
const int patch_pool_size = int(gl_WorkGroupSize.x * gl_WorkGroupSize.y);

const float sqrt3 = 1.73205080757;
const int max_steps = 128;
const float epsilon = 1.0 / 256.0;

const ivec3 p1 = ivec3(
    3145739,
    12582917,
    6291469
);

const ivec3 p2 = ivec3(
    25165843,
    50331653,
    100663319
);

const ivec3 p3 = ivec3(
    904573,
    904577,
    904601
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
    uint _1;

    mat4 eye_transform;

    uint texture_size;
    uint texture_depth;
    uint global_patch_pool_size;
    float _3;
} pc;

layout (binding = 1) buffer patch_buffer     { uint        data[]; } patches_global;
layout (binding = 2) buffer request_buffer   { request_t   data[]; } requests;
layout (binding = 3) buffer lights_buffer    { light_t     data[]; } lights_global;
layout (binding = 4) buffer substance_buffer { substance_t data[]; } substance;
layout (binding = 5) buffer pointer_buffer   { uint        data[]; } pointers;
layout (binding = 6) buffer frustum_buffer   { vec2        data[]; } frustum;
layout (binding = 7) buffer lighting_buffer  { vec4        data[]; } lighting;

shared substance_t substances[gl_WorkGroupSize.x];
shared uint substances_size;

shared light_t lights[gl_WorkGroupSize.x];
shared uint lights_size;

shared uint patch_pool[patch_pool_size];
shared vec4 workspace[gl_WorkGroupSize.x * gl_WorkGroupSize.y];

shared bool test;

vec2 uv(vec2 xy){
    vec2 uv = xy / (gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    uv = uv * 2.0 - 1.0;
    uv.y *= -float(gl_NumWorkGroups.y) / gl_NumWorkGroups.x;
    return uv;
}

uint expected_order(vec3 x){
    vec3 dx = inverse(pc.eye_transform)[3].xyz - x;
    return 10;//9 + uint(dot(dx, dx)) / 5;
}

uint work_group_offset(){
    return (gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * patch_pool_size;
}

float phi(ray_t global_r, substance_t sub, inout intersection_t intersection, inout request_t request){
    ray_t r = ray_t(
        (sub.transform * vec4(global_r.x, 1)).xyz,
        mat3(sub.transform) * global_r.d
    );

    vec3 faces = -sign(r.d) * sub.radius;
    vec3 phis = (faces - r.x) / r.d;
    float phi_aabb = max(phis.x, max(phis.y, phis.z));

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
    uvec2 os_hash = ivec2(order, sub.id) * p3.xy + p2.yz;
    uvec3 x_hash  = x_grid * p1 + p3;

    uint hash = os_hash.x ^ os_hash.y ^ x_hash.x ^ x_hash.y ^ x_hash.z;

    // calculate some useful variables for doing lookups
    uint index = hash % patch_pool_size;
    uint global_index = hash % pc.global_patch_pool_size;

    vec3 cell_position = x_grid * radius * 2;
    bool is_valid = (patch_pool[index] & 0xFFFF) == hash >> 16;

    // if necessary, request more data from CPU
    intersection.local_x = r.x;
    intersection.substance = sub;
    intersection.cell_position = cell_position;
    intersection.index = index;
    intersection.cell_radius = radius;
    intersection.global_index = global_index;

    if (inside_aabb && !is_valid) {
        pointers.data[index + work_group_offset()] = global_index; 

        uint upper_hash = hash >> 16;
        if ((patches_global.data[global_index] & 0xFFFF) != upper_hash){
            request = request_t(cell_position, radius, global_index, upper_hash, sub.id, 1);
        }
    }

    uint data = patch_pool[index];
    vec3 alpha = x_scaled - x_grid;

    vec4 phi = mix(sign(data & vertex_masks[0]), sign(data & vertex_masks[1]), alpha.z);
    phi.xy = mix(phi.xy, phi.zw, alpha.y);
    phi.x = mix(phi.x, phi.y, alpha.x);

    phi.x = 2 * radius * (phi.x - 0.5) * float(is_valid);

    return mix(phi_aabb, phi.x, inside_aabb);
}

intersection_t raycast(ray_t r, inout request_t request){
    uint steps;
    intersection_t i;
    
    i.hit = false;
    i.distance = 0;

    for (steps = 0; !i.hit && steps < max_steps && i.distance < pc.render_distance; steps++){
        float p = pc.render_distance;

        uint substanceID = 0;

        // skip over substances you've already passed
        for (; substanceID < substances_size && i.distance > substances[substanceID].far; substanceID++){}

        for (; !i.hit && substanceID < substances_size; substanceID++){
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

    float dist = length(geometry_i.x - l);

    bool is_clear = 
        shadow_i.substance.id == geometry_i.substance.id || 
        shadow_i.substance.id == ~0 ||
        shadow_i.distance > dist;

    return float(is_clear);
}

vec3 light(uint light_i, intersection_t i, vec3 n, inout request_t request){
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

    return (d + s) * attenuation * shadow * light.colour;
}

uvec4 reduce_to_fit(uint i, bvec4 hits, out uvec4 totals){
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

    totals = min(uvec4(workspace[1023]), gl_WorkGroupSize.xxxx);
    barrier();

    bvec4 mask = lessThanEqual(workspace[i], gl_WorkGroupSize.xxxx) && hits;
    barrier();

    uvec4 result = uvec4(workspace[i]);
    barrier();

    return mix(uvec4(~0), result - 1, mask);
}

vec4 reduce_min(uint i, vec4 value){
    barrier();
    workspace[i] = value;
    barrier();

    if ((i &   1) == 0) workspace[i] = min(workspace[i], workspace[i +   1]);
    barrier();
    if ((i &   3) == 0) workspace[i] = min(workspace[i], workspace[i +   2]);
    barrier();
    if ((i &   7) == 0) workspace[i] = min(workspace[i], workspace[i +   4]);
    barrier();
    if ((i &  15) == 0) workspace[i] = min(workspace[i], workspace[i +   8]);
    barrier();
    if ((i &  31) == 0) workspace[i] = min(workspace[i], workspace[i +  16]);
    barrier();
    if ((i &  63) == 0) workspace[i] = min(workspace[i], workspace[i +  32]);
    barrier();
    if ((i & 127) == 0) workspace[i] = min(workspace[i], workspace[i +  64]);
    barrier();
    if ((i & 255) == 0) workspace[i] = min(workspace[i], workspace[i + 128]);
    barrier();
    if ((i & 511) == 0) workspace[i] = min(workspace[i], workspace[i + 256]);
    barrier();
    
    return min(workspace[0], workspace[512]);
}

vec3 get_ray_direction(vec2 xy){
    vec2 uv = uv(xy);
    return normalize(mat3(pc.eye_transform) * vec3(uv.x, uv.y, pc.focal_depth));
}

bool is_light_visible(light_t l, float near, float far, mat4x3 normals){ 
    vec3 light = l.x - inverse(pc.eye_transform)[3].xyz;
    float r = sqrt(length(l.colour) / epsilon);

    vec4 phis = transpose(normals) * light;
    bool frustum_hit = all(lessThanEqual(phis, vec4(r)));

    float depth = dot(pc.eye_transform[2].xyz, light);
    bool depth_hit = depth >= near - r  && depth <= far + r;

    return l.id != ~0 && frustum_hit && depth_hit;
}

void render(uint i, uint j, substance_t s, uint shadow_index, uint shadow_size){
    request_t request;
    request.status = 0;

    vec3 rx = inverse(pc.eye_transform)[3].xyz;
    vec3 d = get_ray_direction(gl_GlobalInvocationID.xy);

    ray_t r = ray_t(rx, d);
    intersection_t intersection = raycast(r, request);

    barrier();

    float dist = length(lights[gl_LocalInvocationID.x].x - lights[gl_LocalInvocationID.y].x);
    bool is_valid = 
        lights[gl_LocalInvocationID.x].id != ~0 && lights[gl_LocalInvocationID.y].id != ~0 &&
        max(gl_LocalInvocationID.x, gl_LocalInvocationID.y) < lights_size;
        
    vec4 result = reduce_min(i, mix(
        vec4(pc.render_distance), 
        vec4(intersection.distance, -intersection.distance, -dist, 0), 
        bvec4(intersection.hit, intersection.hit, is_valid, false)
    ));
    barrier();

    if (dist == -result.z && is_valid){
        lighting.data[j] = vec4(
            (lights[gl_LocalInvocationID.x].x + lights[gl_LocalInvocationID.y].x) / 2, 
            dist / 2
        );
    }
    
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

    t += vec3(
        k % pc.texture_size,
        (k % (pc.texture_size * pc.texture_size)) / pc.texture_size,
        k / pc.texture_size / pc.texture_size
    );
    t /= vec3(pc.texture_size, pc.texture_size, pc.texture_depth);
    
    vec3 n = 
        inverse(mat3(intersection.substance.transform)) * 
        normalize(texture(normal_texture, t).xyz - 0.5);

    // ambient
    vec3 lighting = vec3(0.25, 0.25, 0.25);

    for (uint light_i = 0; light_i < lights_size; light_i++){
        lighting += light(light_i, intersection, n, request);
    }

    const vec3 sky = vec3(0.5, 0.7, 0.9);
    vec3 hit_colour = texture(colour_texture, t).xyz * lighting;

    vec3 image_colour = mix(sky, hit_colour, intersection.hit);
    image_colour = mix(image_colour, vec3(0, 1, 0), test);
    imageStore(render_texture, ivec2(gl_GlobalInvocationID.xy), vec4(image_colour, 1));

    if (request.status != 0){
        requests.data[request.hash % pc.number_of_calls] = request;
    }
}

bool is_substance_visible(substance_t sub, mat4x3 normals_global){
    mat4x3 normals = mat3(sub.transform) * normals_global;
    vec3 eye = (sub.transform * inverse(pc.eye_transform))[3].xyz;
    vec4 ds = transpose(normals) * eye;

    mat4x3 xs = mat4x3(
        sign(normals[0]) * sub.radius,
        sign(normals[1]) * sub.radius,
        sign(normals[2]) * sub.radius,
        sign(normals[3]) * sub.radius
    );

    vec4 phis = vec4(
        dot(normals[0], xs[0]),
        dot(normals[1], xs[1]),
        dot(normals[2], xs[2]),
        dot(normals[3], xs[3])
    );

    vec3 f = (sub.transform * pc.eye_transform)[2].xyz;
    // vec3 f = mat3(sub.transform) * get_ray_direction(gl_WorkGroupID.xy * gl_WorkGroupSize.xy + gl_WorkGroupSize.xy / 2);
    vec3 v = sign(f) * sub.radius - eye;

    bool is_behind = dot(f, v) < 0;

    bool is_visible = all(greaterThan(phis, ds)) && sub.id != ~0 && sub.near < pc.render_distance && !is_behind;

    if (sub.id == 1 && is_visible){
        // test = true;
    }

    return is_visible;
}

bool is_shadow_visible(substance_t s, float near, float far, vec3 lc, float lr){
    vec3 eye = inverse(pc.eye_transform)[3].xyz;
    vec3 ed = get_ray_direction(gl_WorkGroupSize.xy * gl_WorkGroupID.xy + gl_WorkGroupSize.xy / 2);
    vec3 ec = ed * (near + far) / 2;
    float er = length(ed * far - ec);
    ec += eye;

    vec3 sc = inverse(s.transform)[3].xyz;
    float sr = length(s.radius);

    vec3 d = normalize(lc - ec);
    float alpha = dot(d, sc - ec) / length(lc - ec);
    alpha = clamp(alpha, 0, 1);

    vec3 c = mix(ec, lc, alpha);
    float r = mix(er, lr, alpha);

    return s.id != ~0 && near < far && length(sc - c) < r + sr;
}

void prerender(uint i, uint j, substance_t s, out uint shadow_index, out uint shadow_size){
    mat4x3 rays = mat4x3(
        get_ray_direction( gl_WorkGroupID.xy                * gl_WorkGroupSize.xy),
        get_ray_direction((gl_WorkGroupID.xy + uvec2(1, 0)) * gl_WorkGroupSize.xy),
        get_ray_direction((gl_WorkGroupID.xy + uvec2(1, 1)) * gl_WorkGroupSize.xy),
        get_ray_direction((gl_WorkGroupID.xy + uvec2(0, 1)) * gl_WorkGroupSize.xy)
    );

    mat4x3 normals = mat4x3(
        cross(rays[1], rays[0]), cross(rays[2], rays[1]),
        cross(rays[3], rays[2]), cross(rays[0], rays[3])
    );

    // load shit
    bool directly_visible = is_substance_visible(s, normals);
    light_t l = lights_global.data[i];
    vec2 f = frustum.data[j].xy;
    vec4 li = lighting.data[j];
    barrier();
    bool light_visible = is_light_visible(l, f.x, f.y, normals);
    bool shadow_visible = is_shadow_visible(s, f.x, f.y, li.xyz, li.w);

    // load patches from global memory into shared memory
    uint patch_index = pointers.data[i + work_group_offset()];
    patch_pool[i] = patches_global.data[patch_index];
   
    // visibility check on substances and load into shared memory
    barrier();
    bvec4 hits = bvec4(directly_visible, light_visible, shadow_visible, false);
    uvec4 totals;
    uvec4 indices = reduce_to_fit(i, hits, totals);

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

void main(){
    test = false;
    uint i = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
    uint j = gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x;
    
    substance_t s = substance.data[i];
    uint shadow_index, shadow_size;
    prerender(i, j, s, shadow_index, shadow_size);

    barrier();
    render(i, j, s, shadow_index, shadow_size);
}
