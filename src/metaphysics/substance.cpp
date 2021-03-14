#include "metaphysics/substance.h"

#include <iostream>

using namespace srph;

srph_substance::srph_substance(uint32_t id) {
    _is_asleep = false;
    this->id = id;
}

srph_substance::srph_substance(srph_form * form, srph_matter * matter){
    static uint32_t id = 0;
    _is_asleep = false;
    this->form = *form;
    this->matter = *matter;
    this->id = id++;
}

bool srph_substance::comparator_t::operator()(std::shared_ptr<srph_substance> a, std::shared_ptr<srph_substance> b) const {
    return a->get_id() < b->get_id();
}

uint32_t srph_substance::get_id() const {
    return id;
}

srph_substance::data_t srph_substance::get_data(const vec3_t & eye_position){
    vec3 r;
    srph_bound3_radius(srph_sdf_bound(matter.sdf), r.raw);
    vec3_t eye = matter.to_local_space(eye_position);

    vec3 a = { eye[0], eye[1], eye[2] };
    srph_vec3_abs(&a, &a);

    vec3 x;
    srph_vec3_subtract(&x, &a, &r);
    srph_vec3_max_scalar(&x, &x, 0.0);
    
    float near = srph_vec3_length(&x);
    
    x = { eye[0], eye[1], eye[2] };
    srph_vec3_add(&x, &a, &r);
    
    float far = srph_vec3_length(&x);

    return data_t(
        near, far,
        f32vec3_t(r.x, r.y, r.z),
        id,
        matter.get_matrix()
    );
}

bool srph_substance::data_t::comparator_t::operator()(const srph_substance::data_t & a, const srph_substance::data_t & b) const {
    return a.far < b.far && a.id != static_cast<uint32_t>(~0);
}

srph_substance::data_t::data_t(){
    id = ~0;
}

srph_substance::data_t::data_t(float near, float far, const f32vec3_t & r, uint32_t id, const f32mat4_t & transform){
    this->near = near;
    this->far = far;
    this->r = r;
    this->id = id;
    this->transform = transform;
}
