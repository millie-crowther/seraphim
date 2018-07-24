#ifndef QUATERNION_H
#define QUATERNION_H

#include "vec.h"

class quat_t : public vec4_t {
private:
    quat_t(const std::array<float, 4>& xs);

public:
    quat_t();
    quat_t(const vec4_t& v);

    // accessors
    quat_t conjugate() const;
    quat_t inverse() const;
    quat_t hamilton(const quat_t& q) const;
    vec3_t imaginary_part();
    float real_part();
    mat3_t to_matrix() const;
    vec3_t transform(const vec3_t& v) const; 
    vec3_t inverse_transform(const vec3_t& v) const;

    // operators
    quat_t operator*(const quat_t& q) const;
    quat_t operator*(float s) const;
    vec3_t operator*(const vec3_t& v) const;

    // factories
    static quat_t identity();
    static quat_t angle_axis(float angle, const vec3_t& axis);
    static quat_t from_matrix(const mat3_t& m);
    static quat_t look_at(const vec3_t& forward, const vec3_t& up); 
};

quat_t operator*(float s, const quat_t& q);

#endif
