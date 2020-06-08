
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
