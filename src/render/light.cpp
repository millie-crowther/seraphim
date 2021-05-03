#include "render/light.h"

using namespace srph;

light_t::light_t() { id = ~0; }

light_t::light_t(const f32vec3_t &x, const f32vec4_t &colour) {
    this->x = x;
    this->colour = colour;
    id = 0;
}
