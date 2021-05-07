#include "maths/sdf/platonic.h"

#include <stdlib.h>

static double cuboid_phi(void *data, const vec3 *x) {
    vec3 *r = (vec3 *)data;
    vec3 x1 = *x;

    vec3_abs(&x1, &x1);

    vec3 q;
    vec3_subtract(&q, &x1, r);

    double m = q.x;
    for (int i = 0; i < 3; i++) {
        m = fmax(m, q.v[i]);
    }

    for (int i = 0; i < 3; i++) {
        q.v[i] = fmax(q.v[i], 0.0);
    }

    return vec3_length(&q) + fmin(m, 0.0);
}

double sdf_octahedron(void *data, const vec3 *x) {
    double e = *((double *)data);
    double s = e / sqrt(2);
    vec3 p = *x;
    vec3_abs(&p, &p);

    float m = p.x + p.y + p.z - s;

    vec3 q;
    if (3.0 * p.x < m) {
        q = {{p.x, p.y, p.z}};
    } else if (3.0 * p.y < m) {
        q = {{p.y, p.z, p.x}};
    } else if (3.0 * p.z < m) {
        q = {{p.z, p.x, p.y}};
    } else {
        return m * 0.57735027;
    }

    float k = 0.5 * (q.z - q.y + s);
    k = fmax(k, 0.0);
    k = fmin(k, s);

    vec3 r = {{q.x, q.y - s + k, q.z - k}};
    return vec3_length(&r);
}

void sdf_cuboid_create(uint32_t *id, sdf_t *sdf, vec3 *r) {
    sdf_create(id, sdf, cuboid_phi, r);

    sdf->com = vec3_zero;
    sdf->is_com_valid = true;
}
