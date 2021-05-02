#version 450

struct ray_t {
    vec3 x;
    vec3 d;
};

struct substance_t {
    float near;
    float far;
    uint sdf_id;
    uint material_id;

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
    float cell_radius;
    uint geometry_index;
    uint texture_index;
    vec3 alpha;
    vec3 patch_centre;
};

struct request_t {
    vec3 position;
    float radius;

    uint index;
    uint hash;
    uint substanceID;
    uint status;

    uint sdf_id;
    uint material_id;
    uint _1;
    uint _2;
};

struct light_t {
    vec3 x;
    uint id;

    vec3 colour;
    uint index;
};

struct patch_t {
    uint contents;
    uint hash;
    float phi;
    uint normal;
};

layout (local_size_x = 32, local_size_y = 32) in;

const int work_group_size = int(gl_WorkGroupSize.x * gl_WorkGroupSize.y);
const float sqrt3 = 1.73205080757;
const int max_steps = 128;
const int max_hash_retries = 10;
const float geometry_epsilon = 1.0 / 300.0;

const uint null_status = 0;
const uint geometry_status = 1;
const uint texture_status = 2;

const ivec3 p1 = ivec3(
    904601,
    12582917,
    6291469
);

const ivec3 p2 = ivec3(
    25165843,
    50331653,
    904573
);

const ivec3 p3 = ivec3(
    100663319,
    904577,
    3145739
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
    uint texture_pool_size;

    mat4 eye_transform;

    uint texture_size;
    uint texture_depth;
    uint geometry_pool_size;
    float epsilon;
} pc;

layout (binding = 1) buffer patch_buffer     { patch_t     data[]; } patches;
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

shared vec4 workspace[work_group_size];
shared bool test;

request_t build_request(
    vec3 position,
    float radius,
    uint index,
    uint hash,
    substance_t substance,
    uint status
){
    return request_t(position, radius, index, hash, substance.id, status, substance.sdf_id, substance.material_id, 0, 0);
}

vec2 uv(vec2 xy){
    vec2 uv = xy / (gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    uv = uv * 2.0 - 1.0;
    uv.y *= -float(gl_NumWorkGroups.y) / gl_NumWorkGroups.x;
    return uv;
}

vec3 get_ray_direction(vec2 xy){
    return normalize(mat3(pc.eye_transform) * vec3(uv(xy), pc.focal_depth));
}

int expected_order(vec3 x){
    float dist = length(pc.eye_transform[3].xyz - x);
    float centre = length(uv(gl_GlobalInvocationID.xy));
    const vec2 ks = vec2(1, 2);
    return 10 + int(dot(vec2(dist, centre), ks));
}

uint work_group_offset(){
    return (gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * work_group_size;
}

patch_t get_patch(
    vec3 x, int order, substance_t substance,
    inout intersection_t intersection, inout request_t request,
    out uint hash
){
    float size = geometry_epsilon * order * 2;
    vec3 x_scaled = x / size;
    ivec3 x_grid = ivec3(floor(x_scaled));

    ivec3 x_hash = x_grid * p1 + p2;
    ivec2 os_hash = ivec2(substance.id, order) * p3.xy + p3.yz;
    hash = x_hash.x ^ x_hash.y ^ x_hash.z ^ os_hash.x ^ os_hash.y;

    // calculate some useful variables for doing lookups
    uint index = hash % work_group_size;
    uint geometry_index = hash % pc.geometry_pool_size;

    vec3 cell_position = x_grid * size;
    uvec4 udata = floatBitsToUint(workspace[index]);
    patch_t patch_ =  patch_t(udata.x, udata.y, workspace[index].z, udata.w);

    if (patch_.hash != hash) {
        pointers.data[index + work_group_offset()] = geometry_index;
        patch_ = patches.data[geometry_index];
        if (patch_.hash != hash){
            request = build_request(cell_position, size / 2, geometry_index, hash, substance, geometry_status);
        }
    }

    intersection.cell_radius = size / 2;
    intersection.geometry_index = geometry_index;
    intersection.texture_index = hash % pc.texture_pool_size;
    intersection.alpha = x_scaled - x_grid;
    intersection.patch_centre = cell_position + intersection.cell_radius;

    return patch_;
}

float phi(ray_t global_r, substance_t sub, inout intersection_t intersection, inout request_t request){
    mat4 inv = inverse(sub.transform);
    ray_t r = ray_t(
        (inv * vec4(global_r.x, 1)).xyz,
        mat3(inv) * global_r.d
    );

    vec3 faces = -sign(r.d) * sub.radius;
    vec3 phis = (faces - r.x) / r.d;
    float phi_aabb = max(phis.x, max(phis.y, phis.z));

    // check against outside bounds of aabb
    bool inside_aabb = all(lessThan(abs(r.x), sub.radius));
    phi_aabb = mix(pc.render_distance, phi_aabb, phi_aabb > 0) + pc.epsilon; 

    // find the expected size and order of magnitude of cell
    int order = expected_order(r.x); 
    
    uint hash = ~0;
    patch_t patch_ = patch_t(0, 0, 0, 0);
    
    if (inside_aabb){
        for (int tries = 0; tries < max_hash_retries && hash != patch_.hash; tries++){
            patch_ = get_patch(r.x, order + tries, sub, intersection, request, hash);
        }
    }
    
    intersection.substance = sub;

    vec3 n = vec3((patch_.normal >> uvec3(0, 8, 16)) & 0xFF) / 127.5 - 1;
    float e = dot(intersection.patch_centre - r.x, n) - patch_.phi;
    float phi_plane = min(0, e) / dot(r.d, n);

    float phi = mix(patch_.phi, phi_plane, phi_plane >= 0);
    phi *= float(hash == patch_.hash);

    bool is_empty = patch_.phi > length(vec3(intersection.cell_radius));
    phi = mix(phi.x, patch_.phi, is_empty);

    return mix(phi_aabb, phi, inside_aabb);
}

intersection_t raycast(ray_t r, inout request_t request){
    uint steps;
    intersection_t i;
    
    i.hit = false;
    i.distance = 0;

    uint min_substanceID = 0;

    for (steps = 0; !i.hit && steps < max_steps && i.distance < pc.render_distance; steps++){
        float p = pc.render_distance;
        min_substanceID += int(i.distance > substances[min_substanceID].far);

        for (uint substanceID = min_substanceID; !i.hit && substanceID < substances_size; substanceID++){
            p = min(p, phi(r, substances[substanceID], i, request));
            i.hit = i.hit || p < pc.epsilon;
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
            shadow_i.hit = shadow_i.hit || p < pc.epsilon;
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
    float d = 0.75 * max(pc.epsilon, dot(l, n));

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

    if ((i &   1u) == 0) workspace[i] = min(workspace[i], workspace[i +   1]);
    barrier();
    if ((i &   3u) == 0) workspace[i] = min(workspace[i], workspace[i +   2]);
    barrier();
    if ((i &   7u) == 0) workspace[i] = min(workspace[i], workspace[i +   4]);
    barrier();
    if ((i &  15u) == 0) workspace[i] = min(workspace[i], workspace[i +   8]);
    barrier();
    if ((i &  31u) == 0) workspace[i] = min(workspace[i], workspace[i +  16]);
    barrier();
    if ((i &  63u) == 0) workspace[i] = min(workspace[i], workspace[i +  32]);
    barrier();
    if ((i & 127u) == 0) workspace[i] = min(workspace[i], workspace[i +  64]);
    barrier();
    if ((i & 255u) == 0) workspace[i] = min(workspace[i], workspace[i + 128]);
    barrier();
    if ((i & 511u) == 0) workspace[i] = min(workspace[i], workspace[i + 256]);
    barrier();
    
    return min(workspace[0], workspace[512]);
}

void render(uint i, uint j, substance_t s, uint shadow_index, uint shadow_size){
    request_t request;
    request.status = null_status;

    vec3 rx = pc.eye_transform[3].xyz;
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
    uint k = intersection.geometry_index;
    vec3 t = intersection.alpha * 0.5 + 0.25;

    t += vec3(
        k % pc.texture_size,
        (k % (pc.texture_size * pc.texture_size)) / pc.texture_size,
        k / pc.texture_size / pc.texture_size
    );
    t /= vec3(pc.texture_size, pc.texture_size, pc.texture_depth);
    
    vec3 n = 
        mat3(intersection.substance.transform) * 
        normalize(texture(normal_texture, t).xyz - 0.5);

    // ambient
    vec3 lighting = vec3(0.25, 0.25, 0.25);

    for (uint light_i = 0; light_i < lights_size; light_i++){
        lighting += light(light_i, intersection, n, request);
    }

    const vec3 sky = vec3(0.5, 0.7, 0.9);
    vec3 hit_colour = texture(colour_texture, t).xyz * lighting;

    vec3 image_colour = mix(sky, hit_colour, intersection.hit);

    // debug line:
    // image_colour = mix(image_colour, vec3(0, 1, 0), test);

    imageStore(render_texture, ivec2(gl_GlobalInvocationID.xy), vec4(image_colour, 1));

    if (request.status != null_status){
        requests.data[request.hash % pc.number_of_calls] = request;
    }
}

bool is_light_visible(light_t l, float near, float far, mat4x3 normals){ 
    vec3 light = l.x - pc.eye_transform[3].xyz;
    float r = sqrt(length(l.colour) / pc.epsilon);

    vec4 phis = transpose(normals) * light;
    bool frustum_hit = all(lessThanEqual(phis, vec4(r)));

    float depth = dot(pc.eye_transform[2].xyz, light);
    bool depth_hit = depth >= near - r  && depth <= far + r;

    return l.id != ~0 && frustum_hit && depth_hit;
}

bool is_substance_visible(substance_t sub, mat4x3 normals_global){
    mat4 inv = inverse(sub.transform);
    mat4x3 normals = mat3(inv) * normals_global;
    vec3 eye = (inv * pc.eye_transform[3]).xyz;
    vec4 ds = transpose(normals) * eye;

    vec4 phis = vec4(
        dot(abs(normals[0]), vec3(sub.radius)),
        dot(abs(normals[1]), vec3(sub.radius)),
        dot(abs(normals[2]), vec3(sub.radius)),
        dot(abs(normals[3]), vec3(sub.radius))
    );

    vec3 f = mat3(sub.transform) * get_ray_direction(gl_WorkGroupID.xy * gl_WorkGroupSize.xy + gl_WorkGroupSize.xy / 2);
    
    // TODO: fix this line
    bool is_behind  = false;//all(greaterThan(sub.radius + eye * sign(f), vec3(0)));
    bool is_eye_inside = false;//all(lessThanEqual(eye, sub.radius));

    bool is_visible = 
        sub.id != ~0 && sub.near < pc.render_distance && 
        (all(greaterThan(phis, ds)) || is_eye_inside) && !is_behind;

    return is_visible;
}

bool is_shadow_visible(substance_t s, vec2 view_frustum, vec3 light_position){
    /*

                  __--G--__  r
            x_--``    |    ``--_               S = shadowing_substance
             \        |        /              L = light_position
              \       |       /               G = geometry_centre
               \      |      /                r = geometry_radius
                \     |     /
       _____     \    |    /
      /     \     \   |   /                   o = vector pointing out of screen towards you
     |   S   |     \  |  /
      \_____/       \ | /
                     \|/
                      L
    */

    vec3 eye_position = pc.eye_transform[3].xyz;
    vec3 eye_direction = get_ray_direction(gl_WorkGroupSize.xy * gl_WorkGroupID.xy + gl_WorkGroupSize.xy / 2);
    vec3 view_centre = eye_direction * (view_frustum.x + view_frustum.y) / 2;
    float view_radius = length(eye_direction * view_frustum.y - view_centre);
    view_centre += eye_position;

    vec3 substance_origin = s.transform[3].xyz;
    float substance_radius = length(s.radius);

    vec3 difference = normalize(light_position - view_centre);
    float alpha = dot(difference, substance_origin - view_centre) / length(light_position - view_centre);
    alpha = clamp(alpha, 0, 1);

    vec3 c = mix(view_centre, light_position, alpha);
    float r = mix(view_radius, 0, alpha);

    return s.id != ~0 && view_frustum.x < view_frustum.y && length(substance_origin - c) < r + substance_radius;
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
    light_t l = lights_global.data[i];
    vec2 view_frustum = frustum.data[j].xy;
    vec4 light = lighting.data[j];

    // visibility check on substances and load into shared memory
    barrier();
    bvec4 hits = bvec4(
        is_substance_visible(s, normals),
        is_light_visible(l, view_frustum.x, view_frustum.y, normals),
        is_shadow_visible(s, view_frustum, light.xyz),
        false
    );
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

    // load patches from global memory into shared memory
    patch_t data = patches.data[pointers.data[i]];
    vec3 udata = uintBitsToFloat(uvec3(data.contents, data.hash, data.normal));
    workspace[i] = vec4(udata.x, udata.y, data.phi, udata.z);
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
