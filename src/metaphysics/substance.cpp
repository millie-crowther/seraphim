#include "metaphysics/substance.h"

#include <iostream>

using namespace srph;

srph_substance::srph_substance(uint32_t id) {
    this->id = id;
}

srph_substance::srph_substance(srph_form * form, srph_matter * matter){
    static uint32_t id = 0;
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

srph_substance::data_t srph_substance::get_data(const vec3 * eye_position){
    vec3 r;
    srph_bound3_radius(srph_sdf_bound(matter.sdf), r.v);

    vec3 eye;
    srph_matter_to_local_position(&matter, &eye, eye_position);

    vec3_abs(&eye, &eye);

    vec3 x;
    vec3_subtract(&x, &eye, &r);

    for (int i = 0; i < 3; i++){
        x.v[i] = fmax(x.v[i], 0.0);
    }

    float near = (float) vec3_length(&x);
    
    x = eye;
    vec3_add(&x, &eye, &r);
    
    float far = (float) vec3_length(&x);

    data_t data(
        near, far,
        f32vec3_t(r.x, r.y, r.z),
        id
    );

    srph_matter_transformation_matrix(&matter, data.transform);

    return data;
}

bool srph_substance::data_t::comparator_t::operator()(const srph_substance::data_t & a, const srph_substance::data_t & b) const {
    return a.far < b.far && a.id != static_cast<uint32_t>(~0);
}

srph_substance::data_t::data_t(){
    id = ~0;
}

srph_substance::data_t::data_t(float near, float far, const f32vec3_t & r, uint32_t id){
    this->near = near;
    this->far = far;
    this->r = r;
    this->id = id;
}
