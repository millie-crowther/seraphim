#ifndef SUBSTANCE_H
#define SUBSTANCE_H

#include <memory>

#include "scene/transform.h"
#include "form/form.h"
#include "matter/matter.h"

class substance_t {
public:
    struct data_t {
        float near;
        float far;
        float _1;
        float _2;

        f32vec3_t r;
        uint32_t id;

        f32mat4_t transform;

        data_t();
        data_t(float near, float far, const f32vec3_t & r, uint32_t id, const f32mat4_t & transform);

        struct comparator_t {
            bool operator()(const substance_t::data_t & a, const substance_t::data_t & b) const;
        };
    };

    struct comparator_t {
        bool operator()(const substance_t & a, const substance_t & b) const;
    };

    substance_t(std::shared_ptr<form_t> form, std::shared_ptr<matter_t> matter);

    std::shared_ptr<form_t> get_form() const;
    std::shared_ptr<matter_t> get_matter() const;
    data_t get_data(const vec3_t & eye_position);
    uint32_t get_id() const;

private:
    uint32_t id;
    std::shared_ptr<form_t> form;
    std::shared_ptr<matter_t> matter;
};

#endif
