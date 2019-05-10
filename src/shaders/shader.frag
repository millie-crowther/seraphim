#version 450
#extension GL_ARB_separate_shader_objects : enable

//
// types
//
struct intersection_t {
    bool hit;
    vec3 p;
};

struct ray_t {
    vec3 pos;
    vec3 dir;
    float dist;
    bool hit;
};

struct point_light_t {
    vec3 pos;
    vec4 colour;
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
layout(location = 1) {
    uint structure[10];
} octree;


//
// GLSL inputs
//
in vec4 gl_FragCoord;

float f = 1.0;
float render_distance = 1000.0;
int max_steps = 64;
float epsilon = 0.005;
float shadow_softness = 64;
const uint is_leaf_flag = 1 << 31;
const uint null_node = 0;

node_t octree_lookup(vec3 x){
    node_t node = base_node;
    while (true){
        if (octree.structure[node.i] & is_leaf_flag){
            break;
        }
        //  else if (octree.structure[node.i] == null_node) {
        //     node.is_valid = false;
        //     // TODO: signal CPU for data
        //     found = true;
        // }
         else {
            node.i = octree.structure[node.i];
            node.size /= 2;

            if (x.x > node.min.x + node.size / 2){
                node.min.x += node.size;
                node.i += 1;
            }

            if (x.y > node.min.y + node.size / 2){
                node.min.y += node.size;
                node.i += 2;
            }

            if (x.z > node.min.z + node.size / 2){
                node.min.z += node.size;
                node.i += 4;
            }
        }
    }

    return node;
}

ray_t advance(ray_t r){
    node_t node = octree_lookup(r.pos);
    // advance through aabb
    return r;
}

ray_t raycast(ray_t r){
    for (int i = 0; i < max_steps && !r.hit && r.dist < render_distance; i++){
	    r = advance(r);
    }
    return r;
}

float shadow(vec3 l, vec3 p){
    ray r = raycast(ray_t(l, normalize(p - l), 0, false));
    if (length(r.pos - p) > epsilon * 2){
        return 0.0;
    } else {
        return 1.0;
    }
}

ray_t get_ray(vec2 uv){
    vec3 camera_up = vec3(0, 1, 0);
    vec3 camera_right = vec3(0, 0, -1);
    vec3 camera_position = vec3(0, 0.5, 0);

    vec3 camera_forward = cross(camera_right, camera_up);

    vec3 dir = camera_forward * f;
    dir += camera_up * uv.y;
    dir += camera_right * uv.x;
    dir = normalize(dir);
   
    return ray_t(camera_position, dir, 0, false);
}

vec4 colour(vec3 p){
    if (p.y <= epsilon){
	    return vec4(0.4, 0.8, 0.6, 1.0);
    } else {
        return vec4(0.9, 0.5, 0.6, 1.0);
    }
}

vec4 light(vec3 p){
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
    ray ry = ray_t(pos, normalize(p - pos), 0, false);
    ry = raycast(ry);
    float shadow = shadow(pos, p);

    //diffuse
    vec3 l = normalize(pos - p);
    vec3 n = normal(p);
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
    vec2 pos = uv;
    
    ray r = get_ray(pos);
    r = raycast(r);
    
    if (r.hit){
        out_colour = colour(r.pos) * light(r.pos);
    } else {
        out_colour = sky(); 
    }
}
