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

    bool approx(float a, float b){
	    return std::abs(a - b) < epsilon;
    }
}
