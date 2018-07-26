#ifndef MATHS_H
#define MATHS_H

template <unsigned int N, unsigned int M> class mat_t;
typedef mat_t<3, 3> mat3_t;
typedef mat_t<4, 4> mat4_t;

template <unsigned int N> class vec_t;
typedef vec_t<3> vec3_t;

namespace maths {
    const float pi = 3.14159265359f;
    const float epsilon = 0.000001f;

    float to_radians(float degrees);
    float to_degrees(float radians);

    bool approx(float a, float b);
}

#endif
