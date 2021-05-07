#include "render/call_and_response.h"

using namespace srph;

vec3_t vertices[8] = {vec3_t(0.0, 0.0, 0.0), vec3_t(2.0, 0.0, 0.0),
                      vec3_t(0.0, 2.0, 0.0), vec3_t(2.0, 2.0, 0.0),
                      vec3_t(0.0, 0.0, 2.0), vec3_t(2.0, 0.0, 2.0),
                      vec3_t(0.0, 2.0, 2.0), vec3_t(2.0, 2.0, 2.0)};

static const uint32_t null_status = 0;

request_t::request_t() {
    status = null_status;
}

uint32_t request_geometry_index(const request_t *call) {
    return call->hash % geometry_pool_size;
}

uint32_t request_texture_index(const request_t *call) {
    return call->hash % texture_pool_size;
}

static uint32_t squash(vec3 * x_){
    vec4 x;
    x.xyz = *x_;
    x.w = 0.0;

    uint8_t bytes[4];
    for (int i = 0; i < 4; i++) {
        bytes[i] = (uint8_t)(fmax(0.0, fmin(x.v[i] * 255.0, 255.0)));
    }
    return *(uint32_t *) bytes;
}

void response_geometry(const request_t *request, const substance_t *substance, patch_t *patch) {
    sdf_t *sdf = substance->matter.sdf;

    bound3_t *bound = srph_sdf_bound(sdf);
    vec3 m;
    srph_bound3_midpoint(bound, &m);
    vec3_t p = mat::cast<double>(request->position) - vec3_t(m.x, m.y, m.z);

    uint32_t contains_mask = 0;

    for (int o = 0; o < 8; o++) {
        vec3_t d = p + vertices[o] * request->radius;
        vec3 d1 = {{d[0], d[1], d[2]}};

        if (!sdf_contains(sdf, &d1)) {
            contains_mask |= 1 << o;
        }
    }

    vec3_t c = p + request->radius;
    vec3 c1 = {{c[0], c[1], c[2]}};
    float phi = (float) sdf_distance(sdf, &c1);

    vec3 n1 = sdf_normal(sdf, &c1);
    vec3_divide_f(&n1, &n1, 2);
    vec3_add_f(&n1, &n1, 0.5);
    uint32_t np = squash(&n1);

    uint32_t x_elem = contains_mask << 16;
    *patch = {x_elem, request->hash, phi, np};

}

void response_texture(const request_t *call, substance_t *substance, uint32_t *normals, uint32_t *colours) {
    sdf_t *sdf = substance->matter.sdf;

    bound3_t *bound = srph_sdf_bound(sdf);
    vec3 m;
    srph_bound3_midpoint(bound, &m);
    vec3_t p = mat::cast<double>(call->position) - vec3_t(m.x, m.y, m.z);

    for (int o = 0; o < 8; o++) {
        vec3_t d = p + vertices[o] * call->radius;
        vec3 d1 = {{d[0], d[1], d[2]}};

        vec3 normal = sdf_normal(sdf, &d1);
        vec3_divide_f(&normal, &normal, 2);
        vec3_add_f(&normal, &normal, 0.5);
        normals[o] = squash(&normal);

        material_t mat;
        matter_material(&substance->matter, &mat, NULL);
        vec3 c = mat.colour;
        colours[o] = squash(&c);
    }
}

bool request_is_valid(const request_t *request) {
    return request->status != null_status;
}

