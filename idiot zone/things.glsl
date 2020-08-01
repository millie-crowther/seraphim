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

vec4 fakesort(uint i, vec4 x){
    // TODO: this is fast, which is good, but will take a lot of frames to 
    //       sort a reversed list.
    vec4 temp;

    workspace[i] = x;

    barrier();
    bool is_sorted = x.x <= workspace[i + 1].x;

    if ((i & 1) == 0 && !is_sorted){
        temp = workspace[i];
        workspace[i] = workspace[i + 1];
        workspace[i + 1] = temp;
    }

    barrier();
    is_sorted = workspace[i].x <= workspace[i + 1].x || i == gl_WorkGroupSize.x * gl_WorkGroupSize.y - 1;

    if ((i & 1) == 1 && !is_sorted){
        temp = workspace[i];
        workspace[i] = workspace[i + 1];
        workspace[i + 1] = temp;
    }

    barrier();

    return workspace[i];
}