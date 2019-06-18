#ifndef MAT_H
#define MAT_H

#include "vec.h"

template<class T, uint8_t M, uint8_t N>
class mat_t {
private:
    // fields
    vec_t<vec_t<T, M>, N> xs;

public:
    // constructors
    mat_t(){}

    // accessors
    T frobenius_norm() const {
        T norm(0);
        for (uint8_t i = 0; i < N; i++){
            norm += xs[i].square_norm();
        }

        return norm;
    }

    // factories
    template<class F>
    static mat_t<T, M, N>
    jacobian(const F & f, const vec_t<T, N> & x, const T & delta){
        mat_t<T, M, N> m;
        m.xs = vec_t<vec_t<T, M>, N>::nabla(f, x, delta);
        return m;
    }
};

typedef mat_t<double, 3, 3> mat3_t;

#endif