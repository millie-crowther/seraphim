#ifndef MATHS_H
#define MATHS_H

template <unsigned int N, unsigned int M> class mat_t;
typedef mat_t<4, 4> mat4_t;

namespace maths {
    const float pi = 3.14159265359f;
    const float epsilon = 0.000001f;

    float to_radians(float degrees);
    float to_degrees(float radians);

    mat4_t perspective(float fov, float aspect, float near, float far);

    bool approx(float a, float b);
}

#endif
