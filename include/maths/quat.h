#ifndef QUATERNION_H
#define QUATERNION_H

#include <array>

#include "vec.h"

class quat_t {
private:
    double w, x, y, z;

    void normalise();

public:
    quat_t();
    quat_t(double w, double x, double y, double z);

    // accessors
    quat_t inverse() const;
    quat_t hamilton(const quat_t& q) const;
    vec3_t vector() const;
    double scalar() const;
    vec3_t rotate(const vec3_t& v) const; 

    // operators
    quat_t operator*(const quat_t& q) const;
    vec3_t operator*(const vec3_t& v) const;

    // factories
    static quat_t identity();
    static quat_t angle_axis(double angle, const vec3_t& axis);
};

#endif
