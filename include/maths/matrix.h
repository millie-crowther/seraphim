#ifndef MATHS_VECTOR_H
#define MATHS_VECTOR_H

#include "core/constant.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <type_traits>

template<class T, uint8_t M, uint8_t N>
class matrix_t : public std::array<T, M * N> {
public:
    // constructors
    matrix_t() : matrix_t(T(0)){}

    matrix_t(const T & x){
        this->fill(x); 
    }

    template<class... Xs>
    matrix_t(typename std::enable_if<sizeof...(Xs) + 1 == M * N, T>::type x, Xs... xs) : 
        std::array<T, M * N>({ x, xs...}) {}

    template<class S>    
    matrix_t(const matrix_t<S, M, N> & x){
        std::transform(x.begin(), x.end(), this->begin(), [](const S & s){ return T(s); });
    }

    // vector modifier operators
    void operator+=(const matrix_t<T, M, N> & x){
        std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::plus<T>());
    }

    void operator-=(const matrix_t<T, M, N> & x){
        std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::minus<T>());
    }

    void operator*=(const matrix_t<T, M, N> & x){
        std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::multiplies<T>());
    }

    void operator/=(const matrix_t<T, M, N> & x){
        std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::divides<T>());
    }

    // scalar modifier operators 
    void operator+=(const T & x){
        *this += matrix_t<T, M, N>(x);
    }

    void operator-=(const T & x){
        *this -= matrix_t<T, M, N>(x);
    }

    void operator*=(const T & x){
        *this *= matrix_t<T, M, N>(x);    
    }

    void operator/=(const T & x){
        *this /= matrix_t<T, M, N>(x);
    }

    // vector accessor operators  
    matrix_t<T, M, N> 
    operator+(const matrix_t<T, M, N> & x) const {
        matrix_t<T, M, N> r;
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::plus<T>());
        return r;
    }

    matrix_t<T, M, N> 
    operator-(const matrix_t<T, M, N> & x) const {
        matrix_t<T, M, N> r;
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::minus<T>());
        return r;
    } 

    matrix_t<T, M, N> 
    operator*(const matrix_t<T, M, N> & x) const {
        matrix_t<T, M, N> r;
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::multiplies<T>());
        return r;
    }

    matrix_t<T, M, N> 
    operator/(const matrix_t<T, M, N> & x) const {
        matrix_t<T, M, N> r;
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::divides<T>());
        return r;
    }
    
    // scalar accessor operators
    matrix_t<T, M, N> 
    operator-(const T & x) const {
        return *this - matrix_t<T, M, N>(x);
    }

    matrix_t<T, M, N> 
    operator+(const T & x) const {
        return *this + matrix_t<T, M, N>(x);    
    }

    matrix_t<T, M, N> 
    operator*(const T & x) const {
        return *this * matrix_t<T, M, N>(x);
    }

    matrix_t<T, M, N> 
    operator/(const T & x) const {
        return *this / matrix_t<T, M, N>(x);
    }

    // negation operator
    matrix_t<T, M, N> 
    operator-() const {
        return *this * matrix_t<T, M, N>(T(-1));
    }

    // equality and ordering operators
    bool 
    operator<(const matrix_t<T, M, N> & x) const {
        return std::lexicographical_compare(this->begin(), this->end(), x.begin(), x.end());
    }

    bool 
    operator==(const matrix_t<T, M, N> & x) const {
        return std::equal(this->begin(), this->end(), x.begin());
    }

    bool 
    operator!=(const matrix_t<T, M, N> & x) const {
        return !(x == *this);
    }

    // getters
    matrix_t<T, M, 1>
    get_column(int col){
        matrix_t<T, M, 1> column;
        for (int row = 0; row < M; row++){
            column[row] = (*this)[row][col];
        }
        return column;
    }
};

namespace vec {
    template<class T>
    matrix_t<T, 3, 1> 
    right(){
        return matrix_t<T, 3, 1>(T(1), T(0), T(0));
    }
    
    template<class T>
    matrix_t<T, 3, 1> 
    up(){
        return matrix_t<T, 3, 1>(T(0), T(1), T(0));
    }

    template<class T>
    matrix_t<T, 3, 1> 
    forward(){
        return matrix_t<T, 3, 1>(T(0), T(0), T(1));
    }

    template<class T, uint8_t N>
    T 
    dot(const matrix_t<T, N, 1> & x, const matrix_t<T, N, 1> & y){
        matrix_t<T, N, 1> h = x * y;
        return std::accumulate(h.begin(), h.end(), T(0));
    }
    
    template<class T, uint8_t N>
    T 
    length(const matrix_t<T, N, 1> & x){
        return std::sqrt(dot(x, x));
    }   

    template<class T, uint8_t N>
    matrix_t<T, N, 1> 
    normalise(const matrix_t<T, N, 1> & x){
        T l = length(x);
        return x / (l == T(0) ? T(1) : l);
    }

    template<class T, uint8_t N>
    T 
    volume(const matrix_t<T, N, 1> & x){
        T product = std::accumulate(x.begin(), x.end(), T(1), std::multiplies<T>());
        if constexpr (std::is_unsigned<T>::value){
            return product;
        } else {
            return std::abs(product);
        }
    }

    template<class T>
    matrix_t<T, 3, 1>
    cross(const matrix_t<T, 3, 1> & x, const matrix_t<T, 3, 1> & y){
        return matrix_t<T, 3, 1>(
            x[1] * y[2] - x[2] * y[1],
            x[2] * y[0] - x[0] * y[2],
            x[0] * y[1] - x[1] * y[0]
        );
    }

    template<class T, uint8_t N, class F>
    matrix_t<T, N, 1> 
    grad(const F & f, const matrix_t<T, N, 1> & x){
        matrix_t<T, N, 1> r;
        for (uint8_t i = 0; i < N; i++){
            matrix_t<T, N, 1> axis;
            axis[i] = constant::epsilon;
            r[i] = (f(x + axis) - f(x - axis));
        }
        return r / (2 * constant::epsilon);
    }
}

namespace mat {
    template<class T, uint8_t M, uint8_t N>
    matrix_t<T, M, N>
    clamp(const matrix_t<T, M, N> & x, const matrix_t<T, M, N> & low, const matrix_t<T, M, N> & high){
        auto result = x;
        for (int i = 0; i < M * N; i++){
            result[i] = std::clamp(x[i], low[i], high[i]);
        }
        return result;
    }

    template<class T, uint8_t M, uint8_t N>
    matrix_t<T, M, N>
    min(const matrix_t<T, M, N> & x, const matrix_t<T, M, N> & y){
        matrix_t<T, M, N> r;
        auto f = [](const T & a, const T & b){ return std::min<T>(a, b); };
        std::transform(x.begin(), x.end(), y.begin(), r.begin(), f);
        return r;
    }

    template<class T, uint8_t M, uint8_t N>
    matrix_t<T, M, N>
    max(const matrix_t<T, M, N> & x, const matrix_t<T, M, N> & y){
        matrix_t<T, M, N> r;
        auto f = [](const T & a, const T & b){ return std::max<T>(a, b); };
        std::transform(x.begin(), x.end(), y.begin(), r.begin(), f);
        return r;
    }

    template<class T, uint8_t M, uint8_t N>
    matrix_t<T, M, N>
    max(const matrix_t<T, M, N> & x, const T & y){
        return max(x, matrix_t<T, M, N>(y));
    }
    
    template<class T, uint8_t M, uint8_t N>
    matrix_t<T, M, N>
    abs(const matrix_t<T, M, N> & x){
        matrix_t<T, M, N> r;
        std::transform(x.begin(), x.end(), r.begin(), [](const T & a){ return std::abs(a); });
        return r;
    }

    template<class T>
    T 
    determinant(const matrix_t<T, 3, 3> & a){
        return vec::dot(a.get_column(0), vec::cross(a.get_column(1), a.get_column(2)));
    }

    template<class T>
    matrix_t<T, 3, 3>
    invert(const matrix_t<T, 3, 3> & a){
        matrix_t<T, 3, 3> a1(
            vec::cross(a.get_column(1), a.get_column(2)),
            vec::cross(a.get_column(2), a.get_column(0)),
            vec::cross(a.get_column(0), a.get_column(1))
        );

        T det = determinant(a);
        if (std::abs(det) < constant::epsilon){
            throw std::runtime_error("Error: tried to invert a singular matrix.");
        } else {
            return a1 / det;
        }
    } 
}

template<class T, uint8_t N>
using vec_t = matrix_t<T, N, 1>;

typedef vec_t<int32_t, 2> i32vec2_t;

typedef vec_t<uint8_t, 2> u8vec2_t;
typedef vec_t<uint8_t, 3> u8vec3_t;
typedef vec_t<uint8_t, 4> u8vec4_t;

typedef vec_t<uint16_t, 2> u16vec2_t;
typedef vec_t<uint16_t, 4> u16vec4_t;

typedef vec_t<uint32_t, 2> u32vec2_t;
typedef vec_t<uint32_t, 3> u32vec3_t;
typedef vec_t<uint32_t, 4> u32vec4_t;

typedef vec_t<int32_t, 3> i32vec3_t;

typedef vec_t<float, 2> f32vec2_t;
typedef vec_t<float, 3> f32vec3_t;
typedef vec_t<float, 4> f32vec4_t;

typedef vec_t<double, 2> f64vec2_t;
typedef vec_t<double, 3> f64vec3_t;
typedef vec_t<double, 4> f64vec4_t;

typedef f64vec2_t vec2_t;
typedef f64vec3_t vec3_t;
typedef f64vec4_t vec4_t;

typedef vec_t<vec_t<float, 4>, 4> f32mat4_t;

#endif
