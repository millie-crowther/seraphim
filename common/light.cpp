#include "light.h"

light_t::light_t() { id = ~0; }

light_t::light_t(uint32_t id, vec3f *x, vec4f *colour) {
    this->x = *x;
    this->colour = *colour;
    this->id = id;
}
