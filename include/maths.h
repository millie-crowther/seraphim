#include "mat.h"

namespace maths {
    const float pi = 3.14159265359f;
 
    float to_radians(float degrees);
    float to_degrees(float radians);

    mat4_t perspective(float fov, float aspect, float near, float far);
}
