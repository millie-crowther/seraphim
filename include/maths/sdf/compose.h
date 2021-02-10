#ifndef SERAPHIM_COMPOSE_H
#define SERAPHIM_COMPOSE_H

#include "sdf.h"

struct srph_intersection_sdf : public srph::sdf3_t {
    srph::sdf3_t * _a;
    srph::sdf3_t * _b;
    double phi(const srph::vec3_t & x) override;
};

void srph_create_intersection_sdf(srph_intersection_sdf * i_sdf, srph::sdf3_t * a, srph::sdf3_t * b);

#endif
