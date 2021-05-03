#include "render/call_and_response.h"
#include "core/constant.h"

using namespace srph;

vec3_t vertices[8] = {vec3_t(0.0, 0.0, 0.0), vec3_t(2.0, 0.0, 0.0),
                      vec3_t(0.0, 2.0, 0.0), vec3_t(2.0, 2.0, 0.0),
                      vec3_t(0.0, 0.0, 2.0), vec3_t(2.0, 0.0, 2.0),
                      vec3_t(0.0, 2.0, 2.0), vec3_t(2.0, 2.0, 2.0)};

static const uint32_t null_status = 0;
static const uint32_t geometry_status = 1;
static const uint32_t texture_status = 2;

call_t::call_t() {
    geometry_hash = ~0;
    status = null_status;
}

bool call_t::is_valid() const { return geometry_hash != static_cast<uint32_t>(~0); }

bool call_t::comparator_t::operator()(const call_t &a, const call_t &b) const {
    if (a.substanceID != b.substanceID) {
        return a.substanceID < b.substanceID;
    }

    if (std::abs(a.radius - b.radius) > epsilon) {
        return a.radius < b.radius;
    }

    if (a.position != b.position) {
        return f32vec3_t::comparator_t()(a.position, b.position);
    }

    return false;
}

response_t::response_t() {}

response_t::response_t(const call_t &call, substance_t *substance) {
    sdf_t *sdf = substance->matter.sdf;

    bound3_t *bound = srph_sdf_bound(sdf);
    vec3 m;
    srph_bound3_midpoint(bound, &m);
    vec3_t p = mat::cast<double>(call.position) - vec3_t(m.x, m.y, m.z);

    uint32_t contains_mask = 0;

    for (int o = 0; o < 8; o++) {
        vec3_t d = p + vertices[o] * call.radius;
        vec3 d1 = {{d[0], d[1], d[2]}};

        if (!srph_sdf_contains(sdf, &d1)) {
            contains_mask |= 1 << o;
        }

        vec3 n1 = srph_sdf_normal(sdf, &d1);
        vec3_t n = vec3_t(n1.x, n1.y, n1.z) / 2 + 0.5;

        normals[o] = squash(vec4_t(n, 0.0));

        material_t mat;
        matter_material(&substance->matter, &mat, nullptr);
        vec3 c = mat.colour;
        colours[o] = squash(vec4_t(c.x, c.y, c.z, 0.0));
    }

    vec3_t c = p + call.radius;
    vec3 c1 = {{c[0], c[1], c[2]}};
    float phi = static_cast<float>(sdf_distance(sdf, &c1));

    vec3 n1 = srph_sdf_normal(sdf, &c1);
    vec3_t n = vec3_t(n1.x, n1.y, n1.z) / 2 + 0.5;
    uint32_t np = squash(vec4_t(n, 0.0));

    uint32_t x_elem = contains_mask << 16;
    patch = {x_elem, call.geometry_hash, phi, np};
}

uint32_t response_t::squash(const vec4_t &x) const {
    uint8_t bytes[4];
    for (int i = 0; i < 3; i++) {
        bytes[i] = (uint8_t)(fmax(0.0, fmin(x[i] * 255.0, 255.0)));
    }
    return *reinterpret_cast<uint32_t *>(bytes);
}

uint32_t call_geometry_index(const call_t *call) {
    return call->geometry_hash % geometry_pool_size;
}

uint32_t call_texture_index(const call_t *call) {
    return call->texture_hash % texture_pool_size;
}

bool call_is_geometry(const call_t *self) {
    return (self->status & geometry_status) != 0;
}

bool call_is_texture(const call_t *self) {
    return (self->status & texture_status) != 0;
}
