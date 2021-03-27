#ifndef SERAPHIM_QUATERNION_H
#define SERAPHIM_QUATERNION_H

#include "vector.h"
#include "matrix.h"

#include <array>

typedef struct srph_quat {
    union {
        struct {
            double x;
            double y;
            double z;
            double w;
        };
        double raw[4];
    };
} srph_quat;

static const srph_quat srph_quat_identity = { 0.0, 0.0, 0.0, 1.0 };

void srph_quat_init(srph_quat * q, double x, double y, double z, double w);
void srph_quat_angle_axis(srph_quat * q, double angle, const vec3 * axis);
void srph_quat_rotate_to(srph_quat * q, const vec3 * from, const vec3 * to);
void srph_quat_to_matrix(const srph_quat * q, double * xs);
void srph_quat_inverse(srph_quat * qi, const srph_quat * q);
void srph_quat_rotate(const srph_quat * q, vec3 * qx, const vec3 * x);

namespace srph {
    class quat_t  {
    private:
        // fields
        vec4_t qs;

    public:
        // public constructors
        quat_t();
        quat_t(double w, double x, double y, double z);

        // accessors
        quat_t inverse() const;

        // operators
        template<class T>
        vec_t<T, 3> operator*(const vec_t<T, 3> & x) const {
            return to_matrix() * x;
        }    
    
        quat_t operator*(const quat_t & r) const;

        void operator*=(const quat_t & r);
        double operator[](uint32_t i) const;

        // convertors
        mat3_t to_matrix() const;

        // factories
        static quat_t angle_axis(double angle, const vec3_t& axis);
        static quat_t euler_angles(const vec3_t & e);
    };
}

#endif
