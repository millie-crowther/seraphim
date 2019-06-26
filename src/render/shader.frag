#version 450
#extension GL_ARB_separate_shader_objects : enable

/*
float D_GGX(float NoH, float a) {
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

vec3 F_Schlick(float VoH, vec3 f0) {
    return f0 + (vec3(1.0) - f0) * pow(1.0 - VoH, 5.0);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

float Fd_Lambert() {
    return 1.0 / PI;
}

void BRDF(...) {
    vec3 h = normalize(v + l);

    float NoV = abs(dot(n, v)) + 1e-5;
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);

    // perceptually linear roughness to roughness (see parameterization)
    float roughness = perceptualRoughness * perceptualRoughness;

    float D = D_GGX(NoH, a);
    vec3  F = F_Schlick(LoH, f0);
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness);

    // specular BRDF
    vec3 Fr = (D * V) * F;

    // diffuse BRDF
    vec3 Fd = diffuseColor * Fd_Lambert();

    // apply lighting...
}
*/

//
// constants
//
const uint is_leaf_flag = 1 << 31;
const uint null_node = 0;
const uint structure_size = 11000;
const uint brickset_size  =  5000;

// these ones could be push constants hypothetically
const float f = 1.0;
const float render_distance = 16;
const int max_steps = 64;
const float epsilon = 0.005;
const float shadow_softness = 64;
const float grid_size = 256;

//
// types sent from CPU
//
struct brick_t {
    vec2 n;
    float d;
    uint uv;
};

struct material_t {
    uint rgba;
};

//
// types used in shader
//
struct ray_t {
    vec3 x;
    vec3 d;
};

struct intersection_t {
    bool hit;
    vec3 x;
    vec3 n;
    vec2 uv;
};
intersection_t null_intersection = intersection_t(false, vec3(0), vec3(0), vec2(0));

struct node_t {
    uint i;
    vec3 min;
    float size;
};
node_t invalid_node = node_t(0, vec3(0), -1);
node_t base_node = node_t(0, vec3(-render_distance), render_distance * 2);

// 
// outputs
//
layout(location = 0) out vec4 out_colour;

//
// push constants
//
layout( push_constant ) uniform window_block {
    uvec2 window_size;
    vec2 dummy;             // alignment
    vec3 camera_position;
    float dummy2;           // alignment
    vec3 camera_right;
} push_constants;

//
// inputs from host
//
layout(binding = 1) buffer octree_buffer {
    uint structure[structure_size];
    brick_t brickset[brickset_size];
} octree;

layout(binding = 2) uniform sampler2D texture_sampler;

//
// GLSL inputs
//
in vec4 gl_FragCoord;

bool node_contains(node_t node, vec3 x){
    return 
        x.x >= node.min.x - epsilon && 
        x.y >= node.min.y - epsilon && 
        x.z >= node.min.z - epsilon &&
        x.x <= node.min.x + node.size + epsilon && 
        x.y <= node.min.y + node.size + epsilon && 
        x.z <= node.min.z + node.size + epsilon;

        // TODO: WHY ARENT THESE THE SAME AAARGHGGH
        // all(greaterThanEqual(x, base_node.min)) &&
        // all(lessThan(x, base_node.min + base_node.size));
}

node_t octree_lookup(vec3 x){
    if (!node_contains(base_node, x)){
        return invalid_node;
    }

    node_t node = base_node;

    for (int j = 0; j < 10 && ((octree.structure[node.i] & is_leaf_flag) == 0); j++){
        node.i = octree.structure[node.i];
        node.size /= 2;

        bvec3 octant = greaterThan(x, node.min + node.size);
        node.i += int(octant.x) + (int(octant.y) << 1) + (int(octant.z) << 2);
        node.min += vec3(octant) * node.size;
    }

    return node;
}

intersection_t plane_intersection(ray_t r, vec2 n2, float d){
    vec3 n = vec3(n2, sqrt(1 - dot(n2, n2)));

    // TODO: can probably make this a two liner with some fancy flying
    float dn = dot(r.d, n);
    float lambda = (d - dot(r.x, n)) / (dn + float(dn == 0) * epsilon);
    return intersection_t(lambda >= 0, r.x + lambda * r.d, n, vec2(0));
}

intersection_t raycast(ray_t r){
    node_t node = base_node;

    for (int i = 0; i < max_steps && length(r.x - push_constants.camera_position) < render_distance; i++){
        // TODO: there's enough info here to start the lookup 
        //       halfway through the tree instead of at the start.
        //       will have to check how much time that actually saves
	    node = octree_lookup(r.x);

        if (node.size < 0 || node.i >= structure_size || node.i == 0){
            break;
        }
    
        if (octree.structure[node.i] != is_leaf_flag){
            uint index = octree.structure[node.i] & ~is_leaf_flag;

            if (index <= brickset_size){
                brick_t brick = octree.brickset[index];
                intersection_t i = plane_intersection(r, brick.n, brick.d);
                if (i.hit && node_contains(node, i.x)){
                    float u = float(brick.uv & 65535);
                    float v = float(brick.uv >> 16);
                    i.uv = vec2(u, v) / grid_size + 0.5 / grid_size;
                    
                    return i;
                }
            }
        }
	
        vec3 lambda_i = abs(
            node.min + sign(max(r.d, 0)) * node.size - r.x
        ) / max(abs(r.d), epsilon);

        float lambda = min(lambda_i.x, min(lambda_i.y, lambda_i.z)) + epsilon;
        r.x += r.d * lambda;
    }

    return null_intersection;
}

float shadow(vec3 l, vec3 p){
    intersection_t i = raycast(ray_t(l, normalize(p - l)));
    return float(length(i.x - p) < epsilon * 2);
}

vec4 colour(vec2 uv){
    return texture(texture_sampler, uv);
    // return vec4(0.9, 0.5, 0.6, 1.0);
}

vec4 light(vec3 p, vec3 n){
    //TODO: 1) blinn-phong lighting
    //      2) more complex lighting
    vec3 pos = vec3(1);
    vec4 colour = vec4(3);
    float kd = 0.5;
    float ks = 0.5;
    float shininess = 32;

    // attenuation
    float dist = length(pos - p);
    float attenuation = 1.0 / (dist * dist);

    //ambient 
    vec4 a = vec4(0.5, 0.5, 0.5, 1.0);

    //shadows
    float shadow = shadow(pos, p);

    //diffuse
    vec3 l = normalize(pos - p);

    // since normals are squished into two elements, may need to flip
    // to recover sign
    if (dot(l, n) < 0){
        n = -n;
    }

    vec4 d = kd * dot(l, n) * colour;

    //specular
    vec3 v = normalize(p);
    vec3 r = reflect(l, n);
    vec4 s = ks * pow(max(dot(r, v), epsilon), shininess) * colour;
    return a + (d + s) * attenuation * shadow;
}

vec4 sky(){
    return vec4(0.5, 0.7, 0.9, 1.0);
}

void main(){
    vec2 uv = gl_FragCoord.xy / push_constants.window_size;
    uv = uv * 2.0 - 1.0;
    uv.x *= push_constants.window_size.x;
    uv.x /= push_constants.window_size.y;
    uv.y *= -1;    
    
    vec3 camera_up = vec3(0, 1, 0);
    vec3 camera_right = push_constants.camera_right;
    vec3 camera_position = push_constants.camera_position;

    vec3 camera_forward = cross(camera_right, camera_up);

    vec3 dir = camera_forward * f;
    dir += camera_up * uv.y;
    dir += camera_right * uv.x;
    dir = normalize(dir);
   
    out_colour = sky();

    intersection_t i = raycast(ray_t(camera_position, dir));

    if (i.hit){
        out_colour = colour(i.uv) * light(i.x, i.n);
    }
}



