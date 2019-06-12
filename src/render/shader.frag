#version 450
#extension GL_ARB_separate_shader_objects : enable

//
// constants
//
float f = 1.0;
float render_distance = 8;
int max_steps = 50;
float epsilon = 0.005;
float shadow_softness = 64;
const uint is_leaf_flag = 1 << 31;
const uint null_node = 0;

//
// types
//
struct ray_t {
    vec3 x;
    vec3 d;
};

struct intersection_t {
    bool hit;
    vec3 x;
    vec3 n;
};
intersection_t null_intersection = intersection_t(false, vec3(0), vec3(0));

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
// buffers
//
const uint structure_size = 25000;
const uint geometry_size  = 25000;
layout(binding = 1) buffer octree_buffer {
    uint structure[structure_size];
    vec4 geometry[geometry_size];
} octree;

//
// GLSL inputs
//
in vec4 gl_FragCoord;

bool node_contains(node_t node, vec3 x){
    return 
        x.x >= node.min.x - epsilon && 
        x.y >= node.min.y - epsilon && 
        x.z >= node.min.z - epsilon &&
        x.x < node.min.x + node.size + epsilon && 
        x.y < node.min.y + node.size + epsilon && 
        x.z < node.min.z + node.size + epsilon;

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

intersection_t plane_intersection(ray_t r, vec3 n, float d){
    float dn = dot(r.d, n);
    float lambda = (d - dot(r.x, n)) / min(dn, -epsilon);
    return intersection_t(d <= 0 && lambda >= 0, r.x + lambda * r.d, n);
}

intersection_t raycast(ray_t r){
    node_t node = base_node;

    for (int i = 0; i < max_steps && length(r.x) < render_distance; i++){
        // TODO: there's enough info here to start the lookup 
        //       halfway through the tree instead of at the start.
        //       will have to check how much time that actually saves
	    node = octree_lookup(r.x);

        if (node.size < 0 || node.i >= structure_size || node.i == 0){
            break;
        }
    
        if (octree.structure[node.i] != is_leaf_flag){
            uint index = octree.structure[node.i] & ~is_leaf_flag;

            if (index <= geometry_size){
                vec4 plane = octree.geometry[index];
                intersection_t i = plane_intersection(r, plane.xyz, plane.w);
                if (i.hit && node_contains(node, i.x)){
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

vec4 colour(vec3 p){
    // if (p.y <= epsilon){
	//     return vec4(0.4, 0.8, 0.6, 1.0);
    // } else {
        return vec4(0.9, 0.5, 0.6, 1.0);
    // }
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
    vec4 d = kd * dot(l, n) * colour;

    //specular
    vec3 v = normalize(p);
    vec3 r = reflect(l, n);
    vec4 s = ks * pow(max(dot(r, v), epsilon), shininess) * colour;
    return a + (d + s) * attenuation;// * shadow;
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
        out_colour = colour(i.x) * light(i.x, i.n);
    }
}




