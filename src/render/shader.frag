#version 450
#extension GL_ARB_separate_shader_objects : enable

//
// constants
//
const uint is_leaf_flag = 1 << 31;
const uint null_node = 0;
const uint structure_size = 100000;

// these ones could be push constants hypothetically
const float f = 1.0;
const int max_steps = 64;
const float epsilon = 0.005;
const float shadow_softness = 64;

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
    float render_distance;
    uint grid_size;

    vec3 camera_position;
    float dummy2;           // alignment

    vec3 camera_right;
    float dummy3;

    vec3 camera_up;
    float dummy4;
} push_const;

//
// inputs from host
//
layout(binding = 1) buffer octree_buffer {
    uint structure[structure_size];
} octree;

layout(binding = 2) buffer request_buffer {
    uint n;
    uint requests[255];
} requests;

layout(binding = 3) uniform sampler2D colour_sampler;
layout(binding = 4) uniform sampler2D geometry_sampler;

//
// GLSL inputs
//
in vec4 gl_FragCoord;

node_t base_node(){
    return node_t(0, vec3(-push_const.render_distance), push_const.render_distance * 2);
}

node_t octree_lookup(vec3 x){
    if (
        abs(x).x > push_const.render_distance ||
        abs(x).y > push_const.render_distance ||
        abs(x).z > push_const.render_distance 
    ){
        return invalid_node;
    }

    node_t node = base_node();

    for (int j = 0; j < 10 && ((octree.structure[node.i] & is_leaf_flag) == 0); j++){
        node.i = octree.structure[node.i];
        node.size /= 2;

        bvec3 octant = greaterThan(x, node.min + node.size);
        node.i += int(octant.x) + (int(octant.y) << 1) + (int(octant.z) << 2);
        node.min += vec3(octant) * node.size;
    }

    return node;
}

vec3 normal(vec2 uv){
    return normalize(texture(geometry_sampler, uv).xyz - 0.5);
}

vec2 uv(uint i){
    uint local_u = (i % 256) + 1;
    uint local_v = i / 256;
    return (vec2(local_u, local_v) + 0.5) / push_const.grid_size;
}

intersection_t raycast(ray_t r){
    node_t node = base_node();

    for (int i = 0; i < max_steps; i++){
        // TODO: there's enough info here to start the lookup 
        //       halfway through the tree instead of at the start.
        //       will have to check how much time that actually saves
	    node = octree_lookup(r.x);

        if (node.size < 0 || node.i >= structure_size || node.i == 0){
            break;
        }
    
        if (octree.structure[node.i] != is_leaf_flag){
            uint id = octree.structure[node.i] & 0xFFFFFF;

            vec2 uv = uv(id);
            return intersection_t(
                true, r.x, normal(uv), id, node
            );
        }
	
        vec3 lambda_i = abs(
            node.min + sign(max(r.d, 0)) * node.size - r.x
        ) / max(abs(r.d), epsilon);

        float lambda = min(lambda_i.x, min(lambda_i.y, lambda_i.z)) + epsilon;
        r.x += r.d * lambda;

        if (length(r.x) < 0.1){
            out_colour = vec4(0, 1, 0, 1);
            return null_intersection;
        }
    }

    return null_intersection;
}

float shadow(vec3 l, vec3 p){
    intersection_t i = raycast(ray_t(l, normalize(p - l)));
    return float(length(i.x - p) < epsilon * 2);
}

vec4 colour(vec2 uv){
    return texture(colour_sampler, uv);
}

vec4 phong_light(vec3 light_p, vec3 x, vec2 uv){
    //TODO: 1) blinn-phong lighting
    //      2) more complex lighting
    vec4 colour = vec4(50);
    float kd = 0.5;
    float ks = 0.76;
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
    vec3 n = normal(uv);

    vec4 d = kd * dot(l, n) * colour;

    //specular
    vec3 v = x - push_const.camera_position;
    vec3 right = push_const.camera_right;
    vec3 u = push_const.camera_up;

    // TODO: not sure about order of cross product here??
    v = vec3(dot(v, right), dot(v, u), dot(v, cross(u, right))); 
    v = normalize(v);

    vec3 r = reflect(l, n);
    vec4 s = ks * pow(max(dot(r, v), epsilon), shininess) * colour;
    return a + (d + s) * attenuation * shadow;
}

vec4 light(vec3 p, vec3 x, vec2 uv){
    vec3 l = normalize(p - x);
    return phong_light(p, x, uv);
}

vec4 sky(){
    return vec4(0.5, 0.7, 0.9, 1.0);
}

void main(){
    vec2 camera_uv = gl_FragCoord.xy / push_const.window_size;
    camera_uv = camera_uv * 2.0 - 1.0;
    camera_uv.x *= push_const.window_size.x;
    camera_uv.x /= push_const.window_size.y;
    camera_uv.y *= -1;    
    
    vec3 up = push_const.camera_up;
    vec3 right = push_const.camera_right;
    vec3 x0 = push_const.camera_position;

    vec3 camera_forward = cross(right, up);

    vec3 d = normalize(camera_forward * f + right * camera_uv.x + up * camera_uv.y);
   
    out_colour = sky();

    ray_t r = ray_t(x0, d);
    intersection_t i = raycast(r);

    if (i.hit){
        vec2 uv = uv(i.brick);
        out_colour = colour(uv) * light(vec3(-3, 3, -3), i.x, uv);
    }
}

