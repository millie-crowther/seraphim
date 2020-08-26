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

    mat_t<T, M, N>
    operator*(const T & x){
        for (auto & row : rows) row *= x;
    }

    vec_t<T, Cols> 
    operator*(const vec_t<T, Cols> & x){
        vec_t<T, Cols> r;
        std::transform(rows.begin(), rows.end(), x.begin(), [](const auto & a, const auto & b){
            return vec::dot(a, b);
        });    
        return r;
    }

    vec_t<T, Rows>
    get_column(int col){
        vec_t<T, Rows> column;
        for (int row = 0; row < Rows; row++){
            column[i] = rows[row][col];
        }
        return column;
    } 
};

namespace mat {
    template<class T, uint8_t M, uint8_t N>
    mat_t<T, M, N>
    operator*(const T & x, const mat_t<T, M, N> & a){
        return a * x;
    }

    template<class T>
    T
    determinant(const mat_t<T, 2, 2> & x){
        return a[0][0] * a[1][1] - a[1][0] * a[0][1];
    }

    template<class T>
    T 
    determinant(const mat_t<T, 3, 3> & a){
        return vec::dot(a.get_column(0), vec::cross(a.get_column(1), a.get_column(2)));
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

    template<class T>
    mat_t<T, 3, 3>
    invert(const mat_t<T, 3, 3> & a){
        mat_t<T, 3, 3> a1(
            vec::cross(a.get_column(1), a.get_column(2)),
            vec::cross(a.get_column(2), a.get_column(0)),
            vec::cross(a.get_column(0), a.get_column(1))
        ); 

        T det = determinant(a);
        if (std::abs(det) < constant::epsilon){
            throw std::runtime_error("Error: tried to invert a singular matrix");
        } else {
            return 1.0 / det * a1;
        }
    }
}

#endif
