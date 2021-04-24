#ifndef SERAPHIM_SUBSTANCE_H
#define SERAPHIM_SUBSTANCE_H

#include <memory>

#include "form.h"
#include "matter.h"

#include "maths/matrix.h"

typedef struct srph_substance {
    struct data_t {
        float near;
        float far;
        float _1;
        float _2;

        srph::f32vec3_t r;
        uint32_t id;

        float transform[16];

        data_t();
        data_t(float near, float far, const srph::f32vec3_t & r, uint32_t id);

        struct comparator_t {
            bool operator()(const data_t & a, const data_t & b) const;
        };
    };

    struct comparator_t {
        bool operator()(std::shared_ptr<srph_substance> a, std::shared_ptr<srph_substance> b) const;
    };

    srph_substance();
    srph_substance(form_t *form, srph_matter *matter, uint32_t i);

    data_t get_data(const vec3 *eye_position);
    uint32_t get_id() const;

    uint32_t id;
    form_t form;
    srph_matter matter;
} bsrph_substance;

#endif
