
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

vec3 BRDF(vec3 n, vec3 l, vec3 x, vec3 f0){
    vec3 v = x - push_constants.camera_position;
    vec3 right = push_constants.camera_right;
    vec3 u = push_constants.camera_up;

    // TODO: not sure about order of cross product here??
    v = vec3(dot(v, right), dot(v, u), dot(v, cross(u, right))); 
    v = normalize(v);

    vec3 h = normalize(l + v);
    vec3 brdf = F(l, h, f0) * G(l, v, h) * D(h);
    brdf /= 4 * dot(n, l) * dot(n, v);
    return brdf;
}