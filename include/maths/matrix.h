#ifndef MATRIX_H
#define MATRIX_H

#include "vec.h"

template<class T, uint8_t Rows, uint8_t Cols>
class mat_t {
private:
    vec_t<vec_t<T, Cols>, Rows> rows;

public:
    template<class... Xs>
    mat_t(typename std::enable_if<sizeof...(Xs)+1 == Rows, vec_t<T, Cols>>::type x, Xs... xs) : rows( x, xs ) {}    

    vec_t<T, Cols> &
    operator[](int i){
        return rows[i];
    }

    vec_t<T, Cols> 
    operator*(const vec_t<T, Cols> & x){
        vec_t<T, Cols> r;
        std::transform(rows.begin(), rows.end(), x.begin(), [](const auto & a, const auto & b){
            return vec::dot(a, b);
        });    
        return r;
    } 
};

namespace mat {
    template<class T>
    T
    determinant(const mat_t<T, 2, 2> & x){
        return a[0][0] * a[1][1] - a[1][0] * a[0][1];
    }

    template<class T>
    T 
    determinant(const mat_t<T, 3, 3> & a){
        return 
            a[0][0] * a[1][1] * a[2][2] +
            a[0][1] * a[1][2] * a[2][0] +
            a[0][2] * a[1][0] * a[2][1] -
            a[0][2] * a[1][1] * a[2][0] -
            a[0][1] * a[1][0] * a[2][2] -
            a[0][0] * a[1][2] * a[2][1];
    }   

    template<class T, uint8_t M, uint8_t N>
    bool 
    is_invertible(const mat_t<T, M, N> & a){
        if constexpr (M == N) {
            return std::abs(determinant(a)) > constant::epsilon;
        } else {
            return false;
        }
    }
}

#endif
