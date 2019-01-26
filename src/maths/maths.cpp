#include "maths.h"

#include <cmath>
#include "core/constant.h"

namespace maths {
    float to_radians(float degrees){
        return degrees * constant::pi / 180.0f;
    }

    float to_degrees(float radians){
        return radians / constant::pi * 180.0f;
    }

    bool approx(float a, float b){
	    return std::abs(a - b) < constant::epsilon;
    }
}
