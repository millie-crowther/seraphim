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
    vec3 o;
    vec3 d;
};

// 
// outputs
//
layout(location = 0) out vec4 out_colour;

//
// inputs
//
layout( push_constant ) uniform window_block {
    uvec2 window_size;
} push_constants;

in vec4 gl_FragCoord;

//
// functions
//
// intersection_t
// intersect(ray_t r){

// }

float f = 1.0;
float render_distance = 1000.0;
int max_steps = 64;
float epsilon = 0.005;
float shadow_softness = 64;

struct ray {
    vec3 pos;
    vec3 dir;
    float dist;
    bool hit;
};

struct point_light {
    vec3 pos;
    vec4 colour;
};

float sphere(vec3 p, float r, vec3 c){
    return length(p - c) - r;
}

float plane(vec3 p, vec3 n){
    return dot(p, n);
}

float phi(vec3 p){
    float plane = plane(p, vec3(0, 1, 0));
    float sphere = sphere(p, 0.1, vec3(1, 0.5, 0));
    return min(plane, sphere);
}

vec3 normal(vec3 p){
    vec3 dx = vec3(epsilon, 0, 0); 
    vec3 dy = vec3(0, epsilon, 0); 
    vec3 dz = vec3(0, 0, epsilon);

    return normalize(vec3(
        phi(p + dx) - phi(p - dx),
        phi(p + dy) - phi(p - dy),
        phi(p + dz) - phi(p - dz)
    ));
}

ray advance(ray r){
    float dist = phi(r.pos);
    r.pos += r.dir * dist;
    r.hit = dist <= epsilon;
    r.dist += dist;
    return r;
}

ray raycast(ray r){
    for (int i = 0; i < max_steps && !r.hit && r.dist < render_distance; i++){
	    r = advance(r);
    }
    return r;
}

float shadow(vec3 l, vec3 p){
    ray r = raycast(ray(l, normalize(p - l), 0, false));
    if (length(r.pos - p) > epsilon * 2){
        return 0.0;
    } else {
        return 1.0;
    }
}

ray get_ray(vec2 uv){
    vec3 camera_up = vec3(0, 1, 0);
    vec3 camera_right = vec3(0, 0, -1);
    vec3 camera_position = vec3(0, 0.5, 0);

    vec3 camera_forward = cross(camera_right, camera_up);

    vec3 dir = camera_forward * f;
    dir += camera_up * uv.y;
    dir += camera_right * uv.x;
    dir = normalize(dir);
   
    return ray(camera_position, dir, 0, false);
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
    ray ry = ray(pos, normalize(p - pos), 0, false);
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
