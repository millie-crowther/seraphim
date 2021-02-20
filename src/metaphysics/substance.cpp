#include "metaphysics/substance.h"

#include <iostream>

using namespace srph;

substance_t::substance_t(uint32_t id) {
    this->id = id;
}

substance_t::substance_t(std::shared_ptr<form_t> form, std::shared_ptr<srph_matter> matter){
    static uint32_t id = 0;

    this->form = form;
    this->matter = matter;
    this->id = id++;
}

std::shared_ptr<form_t> substance_t::get_form() const {
    return form;
}

std::shared_ptr<srph_matter> substance_t::get_matter() const {
    return matter;
}

bool substance_t::comparator_t::operator()(std::shared_ptr<substance_t> a, std::shared_ptr<substance_t> b) const {
    return a->get_id() < b->get_id();
}

uint32_t substance_t::get_id() const {
    return id;
}

substance_t::data_t substance_t::get_data(const vec3_t & eye_position){
    vec3 r1;
    srph_bound3_radius(srph_sdf_bound(matter->sdf), r1.raw);
    vec3_t r = vec3_t(r1.x, r1.y, r1.z);
    vec3_t eye = matter->to_local_space(eye_position);

    float near = vec::length(vec::max(vec::abs(eye) - r, 0.0));
    float far  = vec::length(vec::abs(eye) + r);

    return data_t(
        near, far,
        mat::cast<float>(r),
        id,
        *matter->get_matrix()
    );
}

bool substance_t::data_t::comparator_t::operator()(const substance_t::data_t & a, const substance_t::data_t & b) const {
    return a.far < b.far && a.id != static_cast<uint32_t>(~0);
}

substance_t::data_t::data_t(){
    id = ~0;
}

substance_t::data_t::data_t(float near, float far, const f32vec3_t & r, uint32_t id, const f32mat4_t & transform){
    this->near = near;
    this->far = far;
    this->r = r;
    this->id = id;
    this->transform = transform;
}
