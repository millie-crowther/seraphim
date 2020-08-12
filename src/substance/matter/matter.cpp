#include "substance/matter/matter.h"

matter_t::matter_t(std::shared_ptr<sdf3_t> sdf, const vec3_t & colour){
    this->sdf = sdf;
    this->colour = colour;
}

std::shared_ptr<sdf3_t>
matter_t::get_sdf() const {
    return sdf;
}

vec3_t
matter_t::get_colour(const vec3_t & x) const {
    return colour;
}