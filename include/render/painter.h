#ifndef PAINTER_H
#define PAINTER_H

#include "maths/vec.h"

typedef u8vec4_t colour_t;

template<uint8_t D>
class painter_t {
public:
    colour_t colour(const vec_t<double, D> & x) const {
        uint8_t r = std::max(std::min(x[1], 2.0), 0.0) * 127;
        return colour_t(r, 0x20, 0xFF - r, 0xFF);
    }
};

#endif