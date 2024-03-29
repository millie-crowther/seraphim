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
};

struct ray_intersection_t {
    vec3 position;
    float distance;

    vec3 direction;
    uint sdf_id;

    vec3 normal;
    float _2;
};

struct request_t {
    vec3 position;
    float radius;

    uint hash;
    uint sdf_id;
    uint material_id;
    uint status;

    vec3 direction;
    uint _1;
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

struct work_group_persistent_t {
    float near_plane;
    float far_plane;
    float unused1;
    float unused2;
};

layout (local_size_x = 32, local_size_y = 32) in;

const int work_group_size = int(gl_WorkGroupSize.x * gl_WorkGroupSize.y);
const float sqrt3 = 1.73205080757;
const float pi = 3.14159265358979323;
const int max_steps = 128;
const int max_hash_retries = 5;
const float geometry_epsilon = 1.0 / 300.0;

const uint null_request = 0;
const uint geometry_request = 1;
const uint texture_request = 2;
const uint raycast_request = 3;

const ivec4 p1 = ivec4(
    904601,
    12582917,
    6291469,
    3145739
);

const ivec4 p2 = ivec4(
    25165843,
    100663319,
    1025929,
    904573
);

const ivec4 p3 = ivec4(
    904577,
    1268291,
    50331653,
    1020631
);

const uvec4 vertex_masks[2] = {
    uvec4(1 << 16, 1 << 17, 1 << 18, 1 << 19),
    uvec4(1 << 20, 1 << 21, 1 << 22, 1 << 23)
};

layout (binding = 10) uniform writeonly image2D render_texture;
layout (binding = 11) uniform sampler3D normal_texture;
layout (binding = 12) uniform sampler3D colour_texture;
layout (binding = 13) uniform sampler3D physical_texture;

layout(push_constant) uniform push_constants {
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

    uint number_of_raycasts;
    uvec3 _unused;
} pc;

layout (binding = 1) buffer patch_buffer        { patch_t     data[]; } patches;
layout (binding = 2) buffer request_buffer      { request_t   data[]; } requests;
layout (binding = 3) buffer lights_buffer       { light_t     data[]; } lights_global;
layout (binding = 4) buffer substance_buffer    { substance_t data[]; } substance;
layout (binding = 5) buffer pointer_buffer      { uint        data[]; } pointers;
layout (binding = 6) buffer work_group_persistent_buffer { work_group_persistent_t  data[]; } work_group_persistent;
layout (binding = 8) buffer texture_hash_buffer { uint        data[]; } texture_hash;
layout (binding = 9) buffer raycast_buffer      { ray_intersection_t   data[]; } raycasts;

shared substance_t substances[gl_WorkGroupSize.x];
shared uint substances_size;

shared light_t lights[gl_WorkGroupSize.x];
shared uint lights_size;

shared vec4 workspace[work_group_size];
shared bool test;

float expected_size(int order){
    return geometry_epsilon * order * 2;
}

void calculate_cell(vec3 x, int order, out float cell_radius, out vec3 patch_centre){
    float size = expected_size(order);
    cell_radius = size / 2;
    patch_centre = floor(x / size) * size + cell_radius;
}

request_t build_request(substance_t substance, vec3 x, int order, uint hash, uint request_type){
    float cell_radius;
    vec3 patch_centre;
    calculate_cell(x, order, cell_radius, patch_centre);

    vec3 direction;

    request_t result = request_t(
        patch_centre - cell_radius,
        cell_radius, hash, substance.sdf_id, substance.material_id, request_type,
        direction, 0
    );

    return result;
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

uint get_hash(vec3 x, int order, uint id){
    float size = expected_size(order);
    ivec3 x_grid = ivec3(floor(x / size));
    ivec4 hash_vec = ivec4(x_grid, order) * p1 + p2;
    int base_hash = hash_vec.w ^ hash_vec.x ^ hash_vec.y ^ hash_vec.z;
    return base_hash ^ id;
}

uint get_hash2(vec3 x, uint id){
    int order = expected_order(x);
    return get_hash(x, order, id);
}

patch_t get_patch(
    vec3 x, int order, inout intersection_t intersection, inout request_t request, out bool is_patch_found
){
    is_patch_found = true;
    substance_t substance = intersection.substance;
    uint hash = get_hash(x, order, int(substance.sdf_id));

    // calculate some useful variables for doing lookups
    uint index = hash % work_group_size;
    uint geometry_index = hash % pc.geometry_pool_size;

    uvec4 udata = floatBitsToUint(workspace[index]);
    patch_t patch_ = patch_t(udata.x, udata.y, workspace[index].z, udata.w);

    if (patch_.hash != hash) {
        pointers.data[index + work_group_offset()] = geometry_index;
        patch_ = patches.data[geometry_index];
        if (patch_.hash != hash){
            request = build_request(substance, x, order, hash, geometry_request);
            is_patch_found = false;
        }
    }

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
    intersection.substance = sub;

    bool is_patch_found = false;
    float cell_radius;
    vec3 patch_centre;
    if (inside_aabb){
        int tries = 0;
        for (; tries < max_hash_retries && !is_patch_found; tries++){
            patch_ = get_patch(r.x, order + tries, intersection, request, is_patch_found);
        }
        calculate_cell(r.x, order + tries - 1, cell_radius, patch_centre);
    }

    vec3 n = vec3((patch_.normal >> uvec3(0, 8, 16)) & 0xFF) / 127.5 - 1;
    float e = dot(patch_centre - r.x, n) - patch_.phi;
    float phi_plane = min(0, e) / dot(r.d, n);

    float phi = mix(patch_.phi, phi_plane, phi_plane >= 0);
    phi *= float(is_patch_found);

    bool is_empty = patch_.phi > length(vec3(cell_radius));
    phi = mix(phi.x, patch_.phi, is_empty);

    return mix(phi_aabb, phi, inside_aabb);
}

intersection_t raycast(ray_t r, inout request_t request){
    uint steps;
    intersection_t intersection;

    intersection.hit = false;
    intersection.distance = 0;

    uint min_substanceID = 0;

    for (steps = 0; !intersection.hit && steps < max_steps && intersection.distance < pc.render_distance; steps++){
        float p = pc.render_distance;
        min_substanceID += int(intersection.distance > substances[min_substanceID].far);

        for (uint substanceID = min_substanceID; !intersection.hit && substanceID < substances_size; substanceID++){
            p = min(p, phi(r, substances[substanceID], intersection, request));
            intersection.hit = p < pc.epsilon;
        }
        r.x += r.d * p;
        intersection.distance += p;
    }

    intersection.x = r.x;
    return intersection;
}

float shadow_cast(vec3 light_position, vec3 geometry_position, substance_t sub, inout request_t request){
    mat4 inv = inverse(sub.transform);
    light_position = (inv * vec4(light_position, 1)).xyz;
    geometry_position = (inv * vec4(geometry_position, 1)).xyz;

    uint position_hash = get_hash2(geometry_position, sub.sdf_id);

    vec3 light_direction = normalize(geometry_position - light_position);
    uvec3 light_hash = uvec3((light_direction + vec3(1, 3, 5)) * (1 << 9));
    uint hash = position_hash ^ (light_hash.x | light_hash.y | light_hash.z);

//    request = request_t(
//        light_position, 0, hash, sub.sdf_id, sub.sdf_id, raycast_request, light_direction, 0
//    );
    return 1;
}

//float shadow_cast1(vec3 l, uint light_i, intersection_t geometry_i, inout request_t request){
//    uint steps;
//    ray_t r = ray_t(l, normalize(geometry_i.x - l));
//
//    intersection_t shadow_i;
//    shadow_i.substance.id = ~0;
//    shadow_i.hit = false;
//    shadow_i.distance = 0;
//
//    for (steps = 0; !shadow_i.hit && steps < max_steps && shadow_i.distance < pc.render_distance; steps++){
//        float p = pc.render_distance;
//
//        for (uint i = 0; !shadow_i.hit && i < substances_size; i++){
//            p = min(p, phi(r, substances[i], shadow_i, request));
//            shadow_i.hit = p < pc.epsilon;
//        }
//        r.x += r.d * p;
//        shadow_i.distance += p;
//    }
//
//    float dist = length(geometry_i.x - l);
//
//    bool is_clear =
//    shadow_i.substance.id == geometry_i.substance.id ||
//    shadow_i.substance.id == ~0 ||
//    shadow_i.distance > dist;
//
//    return float(is_clear);
//}

float DistributionGGX(vec3 N, vec3 H, float a){
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = pi * denom * denom;

    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float k){
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k){
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0){
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 cook_torrance_brdf(
    vec3 light_position, vec3 light_colour,
    vec3 world_position, vec3 eye_position, vec3 normal,
    float roughness, float metallic, vec3 albedo
){
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 view = normalize(eye_position - world_position);
    vec3 light = normalize(light_position - world_position);
    vec3 half_vector = normalize(view + light);
    float distance = length(light_position - world_position);
    float attentuation = 1.0 / (distance * distance);
    vec3 radiance = light_colour * attentuation;
    float NDF = DistributionGGX(normal, half_vector, roughness);
    float G   = GeometrySmith(normal, view, light, roughness);
    vec3 F    = fresnelSchlick(max(dot(half_vector, view), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, light), 0.0);
    float denominator = 4.0 * max(dot(normal, view), 0.0) * NdotL;
    vec3 specular     = (NDF * G * F) / max(denominator, 0.001);

    return (kD * albedo / pi + specular) * radiance * NdotL;
}

vec3 light(uint light_i, intersection_t i, vec3 n, float roughness, float metallic, vec3 albedo, inout request_t request){
    const float shininess = 16;

    light_t light = lights[light_i];
    vec3 light_position = light.x;
    vec3 light_colour = light.colour;
    vec3 world_position = i.x;
    vec3 eye_position = pc.eye_transform[3].xyz;
    vec3 colour = cook_torrance_brdf(
        light_position, light_colour, world_position, eye_position, n, roughness, metallic, albedo);

    //shadows
    float shadow = shadow_cast(light.x, i.x, i.substance, request);

    return colour * shadow;
}

uvec4 reduce_to_fit(uint i, bvec4 hits, out uvec4 totals){
    vec4 x;
    barrier();
    workspace[i] = uvec4(hits);
    barrier();

    if ((i &   1u) != 0) workspace[i] += workspace[i &   ~1u];
    barrier();
    if ((i &   2u) != 0) workspace[i] += workspace[i &   ~2u |   1u];
    barrier();
    if ((i &   4u) != 0) workspace[i] += workspace[i &   ~4u |   3u];
    barrier();
    if ((i &   8u) != 0) workspace[i] += workspace[i &   ~8u |   7u];
    barrier();
    if ((i &  16u) != 0) workspace[i] += workspace[i &  ~16u |  15u];
    barrier();
    if ((i &  32u) != 0) workspace[i] += workspace[i &  ~32u |  31u];
    barrier();
    if ((i &  64u) != 0) workspace[i] += workspace[i &  ~64u |  63u];
    barrier();
    if ((i & 128u) != 0) workspace[i] += workspace[i & ~128u | 127u];
    barrier();
    if ((i & 256u) != 0) workspace[i] += workspace[i & ~256u | 255u];
    barrier();
    if ((i & 512u) != 0) workspace[i] += workspace[511];
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
    request_t geometry_request;
    geometry_request.status = null_request;

    vec3 rx = pc.eye_transform[3].xyz;
    vec3 d = get_ray_direction(gl_GlobalInvocationID.xy);

    ray_t r = ray_t(rx, d);
    intersection_t intersection = raycast(r, geometry_request);

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

    work_group_persistent.data[j] = work_group_persistent_t(
        result.x,
        -result.y,
        0,
        0
    );
//    frustum.data[j] = vec2(result.x, -result.y);

    barrier();

    substances_size = shadow_size;
    if (shadow_index != ~0){
        substances[shadow_index] = s;
    }

    // find texture coordinate
    mat4 inv = inverse(intersection.substance.transform);
    vec3 x =  (inv * vec4(intersection.x, 1)).xyz;
    int order = expected_order(x) * 2;
    float size = expected_size(order);
    vec3 x_scaled = x / size;
    vec3 alpha = x_scaled - floor(x_scaled);
    uint texture_hash_ = get_hash(x, order, int(intersection.substance.material_id));
    uint texture_index = texture_hash_ % pc.texture_pool_size;
    vec3 t = alpha * 0.5 + 0.25;

    t += vec3(
        texture_index % pc.texture_size,
        (texture_index % (pc.texture_size * pc.texture_size)) / pc.texture_size,
        texture_index / pc.texture_size / pc.texture_size
    );
    t /= vec3(pc.texture_size, pc.texture_size, pc.texture_depth);

    vec3 n = mat3(intersection.substance.transform) * normalize(texture(normal_texture, t).xyz - 0.5);

    // ambient
    vec3 lighting = vec3(0.25, 0.25, 0.25);
    vec3 albedo = texture(colour_texture, t).xyz;
    vec3 physical = texture(physical_texture, t).xyz;
    float metallic = physical.x;
    float roughness = physical.y;

    for (uint light_i = 0; light_i < lights_size; light_i++){
        lighting += light(light_i, intersection, n, roughness, metallic, albedo, geometry_request);
    }

    const vec3 sky = vec3(0.5, 0.7, 0.9);
    vec3 hit_colour = lighting;
    vec3 image_colour = mix(sky, hit_colour, intersection.hit);

    barrier();

    // debug line:
    //    barrier();
    //    image_colour = mix(image_colour, vec3(0, 1, 0), test);
    //    barrier();
    bool texture_data_present = texture_hash.data[texture_index] == texture_hash_;

    if (!intersection.hit || texture_data_present){
        imageStore(render_texture, ivec2(gl_GlobalInvocationID.xy), vec4(image_colour, 1));
    }

    barrier();

    if (intersection.hit && !texture_data_present){
        request_t texture_request = build_request(intersection.substance, x, order, texture_hash_, texture_request);
        requests.data[texture_hash_ % pc.number_of_calls] = texture_request;
    }

    barrier();
    if (geometry_request.status != null_request){
        requests.data[geometry_request.hash % pc.number_of_calls] = geometry_request;
    }
}

bool is_light_visible(light_t l, float near, float far, mat4x3 normals){
    return l.id != ~0;
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
    return s.id != ~0;
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
    work_group_persistent_t persistent = work_group_persistent.data[j];
    vec2 view_frustum = vec2(persistent.near_plane, persistent.far_plane);

    // visibility check on substances and load into shared memory
    barrier();
    bvec4 hits = bvec4(
        is_substance_visible(s, normals),
        is_light_visible(l, view_frustum.x, view_frustum.y, normals),
        is_shadow_visible(s, view_frustum, vec3(0)),
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
