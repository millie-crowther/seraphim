#ifndef PAINTER_H
#define PAINTER_H

#include "maths/vec.h"

typedef u8vec4_t colour_t;

template<uint8_t D>
class painter_t {
public:
    colour_t colour(const vec_t<double, D> & x) const {
        uint8_t g = std::max(std::min(x[1], 1.0), 0.0) * 255;
        return colour_t(0x0A, g, 0x0A, 0xFF);
    }
};

#endif