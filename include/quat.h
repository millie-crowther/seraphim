#ifndef QUATERNION_H
#define QUATERNION_H

#include "vec.h"

class quat_t : public vec4_t {
private:
    quat_t(std::array<float, 4> xs);

public:
    quat_t();
    quat_t(const vec4_t& v);

    quat_t inverse();
    vec3_t conjugate(const vec3_t& v);
    quat_t hamilton(const quat_t& q);
    vec3_t imaginary_part();
    float real_part();
    mat3_t to_matrix();

    static quat_t identity();
    static quat_t angle_axis(float angle, const vec3_t& axis);
    static quat_t from_matrix(const mat3_t& m);
    static quat_t look_at(const vec3_t& forward, const vec3_t& up); 
};

#endif
