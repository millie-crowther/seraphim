#ifndef PAINTER_H
#define PAINTER_H

#include "maths/vec.h"

template<uint8_t D>
class painter_t {
public:
    f32vec4_t colour(const vec_t<double, D> & x) const {
        return f32vec4_t(0.5f, static_cast<float>(x[1]), 0.5f, 1.0f);
    }
};

#endif