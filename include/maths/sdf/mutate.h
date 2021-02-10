#ifndef SERAPHIM_MUTATE_H
#define SERAPHIM_MUTATE_H

#include "maths/sdf/sdf.h"
#include "physics/transform.h"

struct srph_transform_sdf : public srph::sdf3_t {
    srph::transform_t * _transform;
    srph::sdf3_t * _sdf;
    
    double phi(const srph::vec3_t & x) override;
};

void srph_create_transform_sdf(srph_transform_sdf * transform_sdf, srph::transform_t * tf, srph::sdf3_t * sdf);

#endif
