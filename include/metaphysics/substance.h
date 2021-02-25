#ifndef SUBSTANCE_H
#define SUBSTANCE_H

#include <memory>

#include "form.h"
#include "matter.h"

namespace srph {
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
            bool operator()(std::shared_ptr<substance_t> a, std::shared_ptr<substance_t> b) const;
        };

        substance_t(uint32_t i);
        substance_t(srph_form * form, std::shared_ptr<srph_matter> matter);

        std::shared_ptr<srph_matter> get_matter() const;
        data_t get_data(const vec3_t & eye_position);
        uint32_t get_id() const;

    private:
        uint32_t id;
        srph_form * form;
        std::shared_ptr<srph_matter> matter;
    };
}

#endif
