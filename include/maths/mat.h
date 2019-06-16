#ifndef MAT_H
#define MAT_H

#include "vec.h"

template<class T, uint8_t M, uint8_t N>
class mat_t {
private:
    vec_t<vec_t<T, N>, M> xs;

    mat_t(const vec_t<vec_t<T, N>, M> & xs){
        this->xs = xs;
    }

public:
    /*
        factories
    */
    template<class F>
    static mat_t<T, M, N>
    jacobian(const F & f, const T & delta){
        return mat_t<T, M, N>(vec_t<vec_t<T, N>, M>::nabla(f, delta));
    }
};

typedef mat_t<double, 3, 3> mat3_t;

#endif