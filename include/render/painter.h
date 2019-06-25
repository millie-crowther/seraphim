#ifndef PAINTER_H
#define PAINTER_H

#include "maths/vec.h"

typedef u8vec4_t colour_t;

template<uint8_t D>
class painter_t {
public:
    colour_t colour(const vec_t<double, D> & x) const {
        return colour_t(128, 255, 128, 255);
    }
};

#endif