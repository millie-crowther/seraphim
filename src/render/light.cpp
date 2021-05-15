#include "render/light.h"

light_t::light_t() { id = ~0; }

light_t::light_t(vec3f *x, vec4f *colour) {
    this->x = *x;
    this->colour = *colour;
    id = 0;
}
