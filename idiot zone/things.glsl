
void reduce_surface_aabb(uint i, bool hit, vec3 x){
    workspace[i] = mix(vec4(pc.render_distance), x.xyzz, hit);

    barrier();
    if ((i & 0x001) == 0) workspace[i] = min(workspace[i], workspace[i +   1]);
    barrier();
    if ((i & 0x003) == 0) workspace[i] = min(workspace[i], workspace[i +   2]);
    barrier();
    if ((i & 0x007) == 0) workspace[i] = min(workspace[i], workspace[i +   4]);
    barrier();
    if ((i & 0x00F) == 0) workspace[i] = min(workspace[i], workspace[i +   8]);
    barrier();
    if ((i & 0x01F) == 0) workspace[i] = min(workspace[i], workspace[i +  16]);
    barrier();
    if ((i & 0x03F) == 0) workspace[i] = min(workspace[i], workspace[i +  32]);
    barrier();
    if ((i & 0x07F) == 0) workspace[i] = min(workspace[i], workspace[i +  64]);
    barrier();
    if ((i & 0x0FF) == 0) workspace[i] = min(workspace[i], workspace[i + 128]);
    barrier();
    if ((i & 0x1FF) == 0) workspace[i] = min(workspace[i], workspace[i + 256]);
    barrier();

    if (i == 0) surface_min = min(workspace[0], workspace[512]).xyz;

    workspace[i] = mix(vec4(-pc.render_distance), x.xyzz, hit);

    barrier();
    if ((i & 0x001) == 0) workspace[i] = max(workspace[i], workspace[i +   1]);
    barrier();
    if ((i & 0x003) == 0) workspace[i] = max(workspace[i], workspace[i +   2]);
    barrier();
    if ((i & 0x007) == 0) workspace[i] = max(workspace[i], workspace[i +   4]);
    barrier();
    if ((i & 0x00F) == 0) workspace[i] = max(workspace[i], workspace[i +   8]);
    barrier();
    if ((i & 0x01F) == 0) workspace[i] = max(workspace[i], workspace[i +  16]);
    barrier();
    if ((i & 0x03F) == 0) workspace[i] = max(workspace[i], workspace[i +  32]);
    barrier();
    if ((i & 0x07F) == 0) workspace[i] = max(workspace[i], workspace[i +  64]);
    barrier();
    if ((i & 0x0FF) == 0) workspace[i] = max(workspace[i], workspace[i + 128]);
    barrier();
    if ((i & 0x1FF) == 0) workspace[i] = max(workspace[i], workspace[i + 256]);
    barrier();

    if (i == 0) surface_max = max(workspace[0], workspace[512]).xyz;
}

bool is_sphere_visible2(vec3 centre, float radius){
    // centre of work group in screen space coordinates
    uvec2 pxc = gl_WorkGroupID.xy * gl_WorkGroupSize.xy + gl_WorkGroupSize.xy / 2;

    // determine ray at centre of work group
    vec3 b = get_ray_direction(pxc);

    // calculate position of sphere relative to camera
    vec3 a = centre - pc.camera_position;

    // project sphere position into ray, remove values behind camera
    float a1 = max(epsilon, dot(a, b));

    // find closest point to sphere on ray
    vec3 x = pc.camera_position + b * a1;

    // find distance of sphere from ray
    float d = length(centre - x) - radius;

    // subtract the size of the work group cone at this distance
    d -= (gl_WorkGroupSize.x / 2) * a1 / pc.focal_depth;

    return d < 0;
}