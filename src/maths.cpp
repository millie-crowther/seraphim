#include "maths.h"

#include <cmath>
#include "mat.h"

namespace maths {
    float to_radians(float degrees){
        return degrees * pi / 180.0f;
    }

    float to_degrees(float radians){
        return radians / pi * 180.0f;
    }

    mat4_t perspective(float fov, float aspect, float near, float far){
        float tan_f = std::tan(fov / 2.0f);

        return mat4_t({
            1.0f / (aspect * tan_f), 0, 0, 0,
            0, -1.0f / tan_f, 0, 0,
            0, 0, (far + near) / (near - far), 2 * far * near / (near - far),
            0, 0, -1, 0
        });
    }

    bool approx(float a, float b){
	return std::abs(a - b) < epsilon;
    }
}
