#ifndef QUATERNION_H
#define QUATERNION_H

#include <array>

#include "vec.h"

class quat_t {
private:
    // fields
    double w, x, y, z;

    // private constructor
    // (used to restrict unnormalised quaternions to internals of class)
    quat_t(double w, double x, double y, double z, bool should_normalise);

    // private functions
    void normalise();

public:
    // public constructors
    quat_t();
    quat_t(double w, double x, double y, double z);

    // accessors
    quat_t inverse() const;
    quat_t hamilton(const quat_t& q) const;
    vec3_t vector() const;
    double scalar() const;
    vec3_t rotate(const vec3_t& v) const; 

    // operators
    void operator*=(const quat_t & q);
    quat_t operator*(const quat_t& q) const;
    vec3_t operator*(const vec3_t& v) const;

    // factories
    static quat_t identity();
    static quat_t angle_axis(double angle, const vec3_t& axis);
    static quat_t look_at(const vec3_t & forward, const vec3_t & up);
};

#endif
