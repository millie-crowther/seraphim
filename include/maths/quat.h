#ifndef QUATERNION_H
#define QUATERNION_H

#include <array>

#include "vec.h"

class quat_t  {
private:
    // fields
    vec4_t q;

public:
    // public constructors
    quat_t();
    quat_t(double w, double x, double y, double z);

    // accessors
    quat_t inverse() const;

    // operators
    quat_t operator*(const quat_t & q) const;

    // factories
    static quat_t angle_axis(double angle, const vec3_t& axis);
};

#endif
