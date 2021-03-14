#ifndef SUBSTANCE_H
#define SUBSTANCE_H

#include <memory>

#include "form.h"
#include "matter.h"

typedef struct srph_substance {
    struct data_t {
        float near;
        float far;
        float _1;
        float _2;

        srph::f32vec3_t r;
        uint32_t id;

        srph::f32mat4_t transform;

        data_t();
        data_t(float near, float far, const srph::f32vec3_t & r, uint32_t id, const srph::f32mat4_t & transform);

        struct comparator_t {
            bool operator()(const data_t & a, const data_t & b) const;
        };
    };

    struct comparator_t {
        bool operator()(std::shared_ptr<srph_substance> a, std::shared_ptr<srph_substance> b) const;
    };

    srph_substance(uint32_t i);
    srph_substance(srph_form * form, srph_matter * matter);

    data_t get_data(const srph::vec3_t & eye_position);
    uint32_t get_id() const;

    bool _is_asleep;
    uint32_t id;
    srph_form form;
    srph_matter matter;
} srph_substance;

#endif
