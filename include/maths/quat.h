#ifndef QUATERNION_H
#define QUATERNION_H

#include "matrix.h"

#include <array>

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
