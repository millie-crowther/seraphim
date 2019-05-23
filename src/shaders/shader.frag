#version 450
#extension GL_ARB_separate_shader_objects : enable

//
// types
//
struct intersection_t {
    bool hit;
    vec3 x;
    vec3 n;
};

struct ray_t {
    vec3 pos;
    vec3 dir;
    float dist;
};

struct node_t {
    uint i;
    vec3 min;
    float size;
};

// 
// outputs
//
layout(location = 0) out vec4 out_colour;

//
// push constants
//
layout( push_constant ) uniform window_block {
    uvec2 window_size;
} push_constants;

//
// buffers
//
layout(binding = 1) buffer octree_buffer {
    uint structure[100000];
} octree;

//
// GLSL inputs
//
in vec4 gl_FragCoord;

float f = 1.0;
float render_distance = 10;
int max_steps = 64;
float epsilon = 0.005;
float shadow_softness = 64;
const uint is_leaf_flag = 1 << 31;
const uint null_node = 0;

node_t base_node = node_t(0, vec3(-render_distance), render_distance * 2);

node_t octree_lookup(vec3 x){
    node_t node = base_node;
    while (true){
        if ((octree.structure[node.i] & is_leaf_flag) != 0){
            break;
        } else {
            node.i = octree.structure[node.i];
            node.size /= 2;

            bvec3 octant = greaterThan(x, node.min + node.size);
            node.i += int(octant.x) + int(octant.y) << 1 + int(octant.z) << 2;
            node.min += vec3(octant) * node.size;
        }
    }

    return node;
}

intersection_t raycast(ray_t r){
    node_t node;

    // for (int i = 0; i < max_steps && r.dist < render_distance; i++){
	//     node = octree_lookup(r.pos);
    
    //     if ((octree.structure[node.i] & is_leaf_flag) != 0){
    //         // calculate normal for cube 
    //         vec3 d = r.pos - (node.min + node.size / 2);
    //         vec3 ad = abs(d);
    //         vec3 n = vec3(equal(ad, vec3(max(ad.x, max(ad.y, ad.z))))) * sign(d);
            
    //         return intersection_t(true, r.pos, n);
    //     }
	
    //     vec3 lambda_i = (
    //         node.min - r.pos +
    //         vec3(greaterThan(r.dir, vec3(0))) * node.size 
    //     ) / (
    //         r.dir + vec3(equal(r.dir, vec3(0))) * epsilon
    //     );

    //     float lambda = min(lambda_i.x, min(lambda_i.y, lambda_i.z)) + epsilon;
    //     r.pos += r.dir * lambda;
    //     r.dist += lambda;
    // }

    return intersection_t(false, vec3(0), vec3(0));
}

float shadow(vec3 l, vec3 p){
    intersection_t i = raycast(ray_t(l, normalize(p - l), 0));
    return float(length(i.x - p) < epsilon * 2);
}

vec4 colour(vec3 p){
    if (p.y <= epsilon){
	    return vec4(0.4, 0.8, 0.6, 1.0);
    } else {
        return vec4(0.9, 0.5, 0.6, 1.0);
    }
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

    // //diffuse
    vec3 l = normalize(pos - p);
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
    vec3 camera_right = vec3(0, 0, -1);
    vec3 camera_position = vec3(0, 0.5, 0);

    vec3 camera_forward = cross(camera_right, camera_up);

    vec3 dir = camera_forward * f;
    dir += camera_up * uv.y;
    dir += camera_right * uv.x;
    dir = normalize(dir);
   
    intersection_t i = raycast(ray_t(camera_position, dir, 0));
    
    if (i.hit){
        out_colour = colour(i.x) * light(i.x, i.n);
    } else {
        out_colour = sky(); 
    }
}


