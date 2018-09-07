#ifndef MATHS_H
#define MATHS_H

namespace maths {
    const float pi = 3.14159265359f;
    const float epsilon = 0.000001f;

    float to_radians(float degrees);
    float to_degrees(float radians);

    bool approx(float a, float b);
}

#endif
