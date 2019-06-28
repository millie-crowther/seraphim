#version 450
#extension GL_ARB_separate_shader_objects : enable

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

struct node_t {
    uint i;
    vec3 min;
    float size;
};
node_t invalid_node = node_t(0, vec3(0), -1);
node_t base_node = node_t(0, vec3(-render_distance), render_distance * 2);

struct intersection_t {
    bool hit;
    vec3 x;
    vec3 n;
    uint brick;
    node_t node;
};
intersection_t null_intersection = intersection_t(
    false, vec3(0), vec3(0), 0, 
    node_t(0, vec3(0), 0)
);

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
    float dummy3;
    vec3 camera_up;
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

intersection_t plane_intersection(ray_t r, uint i){
    brick_t b = octree.brickset[i];
    vec3 n = vec3(b.n, sqrt(1 - dot(b.n, b.n)));
    float dn = dot(r.d, n);
    float lambda = (b.d - dot(r.x, n)) / (dn + float(dn == 0) * epsilon);
    return intersection_t(lambda >= 0, r.x + lambda * r.d, n, i, base_node);
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
                intersection_t i = plane_intersection(r, index);
                if (i.hit && node_contains(node, i.x)){
                    i.node = node;
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

vec2 uv(intersection_t i){
    brick_t brick = octree.brickset[i.brick];

    // find centre
    uint u = brick.uv & 65535;
    uint v = brick.uv >> 16;
    vec2 uv = vec2(u, v) / grid_size + 0.5 / grid_size;

    // find offset
    vec2 d = vec2(0);

    return uv + d;
}

vec4 colour(intersection_t i){
    vec2 uv = uv(i);

    vec3 c = i.node.min + i.node.size / 2;
    vec3 dx = i.x - c; 

    vec3 v = mix(vec3(1, 0, 0), vec3(0, 1, 0), float(abs(i.n.y) <= 1 - epsilon));
    vec3 u_axis = cross(v, i.n);
    vec3 v_axis = cross(i.n, u_axis);

    vec2 du = vec2(dot(dx, u_axis), dot(dx, v_axis)) / grid_size / i.node.size / 2;

    return texture(texture_sampler, uv + du);
}

vec4 phong_light(vec3 light_p, vec3 x, vec3 n){
    //TODO: 1) blinn-phong lighting
    //      2) more complex lighting
    vec4 colour = vec4(3);
    float kd = 0.5;
    float ks = 0.5;
    float shininess = 32;

    // attenuation
    float dist = length(light_p - x);
    float attenuation = 1.0 / (dist * dist);

    //ambient 
    vec4 a = vec4(0.5, 0.5, 0.5, 1.0);

    //shadows
    float shadow = shadow(light_p, x);

    //diffuse
    vec3 l = normalize(light_p - x);

    // since normals are squished into two elements, may need to flip
    // to recover sign
    if (dot(l, n) < 0){
        n = -n;
    }

    vec4 d = kd * dot(l, n) * colour;

    //specular
    vec3 v = x - push_constants.camera_position;
    vec3 right = push_constants.camera_right;
    vec3 u = push_constants.camera_up;

    // TODO: not sure about order of cross product here??
    v = vec3(dot(v, right), dot(v, u), dot(v, cross(u, right))); 
    v = normalize(v);

    vec3 r = reflect(l, n);
    vec4 s = ks * pow(max(dot(r, v), epsilon), shininess) * colour;
    return a + (d + s) * attenuation * shadow;
}

vec3 F(vec3 l, vec3 h, vec3 f0){
    // schlick approximation
    return f0 + (1 - f0) * pow(1 - dot(l, h), 5);
}

float G1(vec3 x, vec3 h){
    // TODO
    return 0;
}

float G(vec3 l, vec3 v, vec3 h){
    // Smith formulation
    return G1(l, h) * G1(v, h);
}

float D(vec3 h){
    // TODO
    return 0;
}

vec3 BRDF(vec3 n, vec3 l, vec3 v, vec3 f0){
    vec3 h = normalize(l + v);
    vec3 brdf = F(l, h, f0) * G(l, v, h) * D(h);
    brdf /= 4 * dot(n, l) * dot(n, v);
    return brdf;
}

vec4 light(vec3 p, vec3 x, vec3 n){
    vec3 l = normalize(p - x);

    // since normals are squished into two elements, may need to flip
    // to recover sign
    if (dot(l, n) < 0){
        n = -n;
    }

    
    return phong_light(p, x, n);
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
    
    vec3 camera_up = push_constants.camera_up;
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
        out_colour = colour(i) * light(vec3(1.2), i.x, i.n);
    }
}

