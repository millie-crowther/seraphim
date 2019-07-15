#version 450
#extension GL_ARB_separate_shader_objects : enable

//
// constants
//
const uint is_leaf_flag = 1 << 31;
const uint is_empty_flag = 1 << 30;
const uint structure_size = 25000;
const uint requests_size = 64;
const uint brick_id_mask = 0xFFFFFF;
const float sqrt3 = 1.73205080757;

// these ones could be push constants hypothetically
const float f = 1.0;
const int max_steps = 128;
const float epsilon = 1.0 / 256.0;
const float sigma = 64; // TODO: relate this to camera FOV
const float shadow_softness = 64;

//
// types 
//
struct request_t {
    vec3 x;
    uint i;
};

struct octree_node_t {
    uint a;
    uint b;
    uint c;
    uint d;
};

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
    node_t node;
    uint i;
};
intersection_t null_intersection = intersection_t(
    false, vec3(0), vec3(0), 
    node_t(0, vec3(0), 0), 0
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
    uint dummy;

    vec3 camera_position;
    float dummy2;        

    vec3 camera_right;
    float dummy3;

    vec3 camera_up;
    float dummy4;
} push_const;

//
// buffers
//
layout(binding = 1) readonly buffer octree_buffer {
    octree_node_t structure[structure_size];
} octree;

layout(binding = 2) buffer request_buffer {
    request_t requests[requests_size];
} requests;

//
// GLSL inputs
//
in vec4 gl_FragCoord;

void request_buffer_push(vec3 x){
    uint i = uint(dot(x, x)) & (requests_size - 1);
    requests.requests[i] = request_t(x, 1);
}

node_t base_node(){
    return node_t(0, vec3(-push_const.render_distance), push_const.render_distance * 2);
}

bool should_request(uint i, vec4 aabb){
    // TODO: get rid of sqrt here
    return aabb.w >  epsilon * max(1, sigma * length(aabb.xyz + aabb.w / 2 - push_const.camera_position));
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

    while ((octree.structure[node.i].a & is_leaf_flag) == 0){
        node.i = octree.structure[node.i].c;
        node.size /= 2;

        bvec3 octant = greaterThan(x, node.min + node.size);
        node.i += int(octant.x) + (int(octant.y) << 1) + (int(octant.z) << 2);
        node.min += vec3(octant) * node.size;
    }

    return node;
}

vec3 normal(uint i){
    uint data = octree.structure[i].b;
    return vec3(
        data & 0xFF, (data >> 8) & 0xFF, (data >> 16) & 0xFF
    ) / 127.5 - 1;
}

intersection_t plane_intersection(ray_t r, node_t node, uint i){
    vec3 n = normal(i);

    float phi = float(octree.structure[i].b >> 24) / 255.0 - 0.5;

    phi *= sqrt3 * node.size;

    vec3 c = node.min + node.size / 2;
    float d = dot(c, n) - phi;

    float xn = dot(r.x, n);

    if (xn < d){
        return intersection_t(true, r.x, n, node, i);
    }

    float dn = dot(r.d, n);
    float lambda = (d - xn) / (dn + float(dn == 0) * epsilon);

    return intersection_t(lambda >= 0, r.x + lambda * r.d, n, node, i);
}

intersection_t raycast(ray_t r){
    node_t node = base_node();


    for (int i = 0; i < max_steps; i++){
        // TODO: there's enough info here to start the lookup 
        //       halfway through the tree instead of at the start.
        //       will have to check how much time that actually saves
	    node = octree_lookup(r.x);

        if (node.size < 0 || node.i >= structure_size){
            return null_intersection;
        }
    
        if ((octree.structure[node.i].a & is_empty_flag) == 0){
            if (should_request(node.i, vec4(node.min, node.size))){
                request_buffer_push(node.min + node.size / 2);
            }

            // vec3 n = normal(node.i);
            // return intersection_t(true, r.x, n, node);

            intersection_t i = plane_intersection(r, node, node.i);
            if (
                i.hit &&
                i.x.x >= node.min.x && i.x.y >= node.min.y && i.x.z >= node.min.z &&
                i.x.x <= node.min.x + node.size && i.x.y <= node.min.y + node.size &&
                i.x.z <= node.min.z + node.size    
            ){
                return i;
            }
        }
	
        vec3 lambda_i = abs(
            node.min + sign(max(r.d, 0)) * node.size - r.x
        ) / max(abs(r.d), epsilon);

        float lambda = min(lambda_i.x, min(lambda_i.y, lambda_i.z)) + epsilon;
        r.x += r.d * lambda;

        if (length(r.x) < 0.1){
            out_colour = vec4(1, 0, 1, 1);
            return null_intersection;
        }
    }

    return null_intersection;
}

float shadow(vec3 l, vec3 p){
    intersection_t i = raycast(ray_t(l, normalize(p - l)));
    return float(length(i.x - p) < epsilon * 2);
}

vec4 colour(uint i){
    uint data = octree.structure[i].c;
    return vec4(
        data & 0xFF, (data >> 8) & 0xFF, (data >> 16) & 0xFF, data >> 24
    ) / 255.0;
}

vec4 phong_light(vec3 light_p, vec3 x, uint i){
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
    vec3 n = normal(i);

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

vec4 light(vec3 p, vec3 x, uint i){
    vec3 l = normalize(p - x);
    return phong_light(p, x, i);
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
        out_colour = colour(i.i) * light(vec3(-3, 3, -3), i.x, i.i);
    }
}



