#include "render/matter.h"

matter_t::matter_t(const vec3_t & colour){
    this->colour = colour;
}

vec3_t
matter_t::get_colour(const vec3_t & x) const {
    return colour;
}