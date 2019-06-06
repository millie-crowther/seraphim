#ifndef MATHS_VECTOR_H
#define MATHS_VECTOR_H

#include <cmath>
#include "maths.h"
#include <array>
#include <algorithm>
#include "core/constant.h"
#include <iostream>

template<class vec_type_t, unsigned int N>
class vec_t {
protected:
    std::array<vec_type_t, N> xs;

public:
    vec_t() : vec_t(0){}

    vec_t(vec_type_t x){
        xs.fill(x); 
    }

    template<class... Xs>
    vec_t(typename std::enable_if<sizeof...(Xs)+1 == N, vec_type_t>::type x, Xs... _xs) : xs({ x, _xs...}) {}

    template<class T=vec_t<vec_type_t, 3>>
    typename std::enable_if<N == 3, T>::type
    tangent() const {
        return vec_t<vec_type_t, 3>(); // TODO
    } 

    /*
       norms
    */
    vec_type_t square_norm() const {
        return (*this) * (*this);
    }

    vec_type_t norm() const {
        return std::sqrt(square_norm());
    }   

    vec_type_t chebyshev_norm() const {
        vec_type_t max = std::abs(xs[0]);
        for (int i = 1; i < N; i++){
            max = std::max(std::abs(xs[i]), max);
        }
        return max;
    }

    vec_t<vec_type_t, N> normalise() const {
        return *this * maths::inverse_square_root(square_norm());
    }

    /*
        maths
    */
    double angle(const vec_t<vec_type_t, N> & v){
        return std::acos(
            dot(v) * 
            maths::inverse_square_root(square_norm()) * 
            maths::inverse_square_root(v.square_norm())
        );
    }

    vec_t<vec_type_t, N> abs() const {
        vec_t<vec_type_t, N> abs_xs;

        for (int i = 0; i < N; i++){
            abs_xs[i] = std::abs(xs[i]);
        }

        return abs_xs;
    }
 
    /*
       overloaded operators
    */
    vec_type_t operator*(const vec_t<vec_type_t, N> & o) const {
        vec_type_t result = 0;
        for (int i = 0; i < N; i++){
            result += xs[i] * o.xs[i];
        }
        return result;
    }

    vec_type_t operator[](int i) const {
        return xs[i];
    }

    vec_type_t & operator[](int i){
        return xs[i];
    }

    vec_t<vec_type_t, N> operator+(const vec_t<vec_type_t, N>& v) const {
        auto sum = *this;
        for (int i = 0; i < N; i++){
            sum.xs[i] += v.xs[i];
        }
        return sum;
    }

    void operator+=(const vec_t<vec_type_t, N> & v){
        xs = (*this + v).xs;
    }

    void operator*=(vec_type_t scale){
        xs = (*this * scale).xs;
    }

    void operator/=(vec_type_t scale){
        *this *= 1.0 / scale;
    }
  
    vec_t<vec_type_t, N> operator-(const vec_t<vec_type_t, N> & v) const {
         return (*this) + (-v);
    } 

    vec_t<vec_type_t, N> operator-() const {
        return *this * -1;
    }

    vec_t<vec_type_t, N> operator*(vec_type_t scale) const {
        auto product = *this;
        for (int i = 0; i < N; i++){
            product.xs[i] *= scale;
        }
        return product;
    }

    bool operator==(const vec_t<vec_type_t, N> & v) const {
        return (*this - v).square_norm() <= constant::epsilon * constant::epsilon;
    }

    vec_t<vec_type_t, N> operator/(vec_type_t scale) const {
        return *this * (1.0 / scale);
    }

    template<class T=vec_t<vec_type_t, 3>>
    typename std::enable_if<N == 3, T>::type
    operator%(const vec_t<vec_type_t, 3> & v) const {
        return vec_t<vec_type_t, 3>(
            xs[1] * v.xs[2] - xs[2] * v.xs[1],
            xs[2] * v.xs[0] - xs[0] * v.xs[2],
            xs[0] * v.xs[1] - xs[1] * v.xs[0]
        );
    }

    /*
        factories
    */
    vec_t<vec_type_t, N> zero(){
        return vec_t<vec_type_t, N>();
    }
};

typedef vec_t<uint32_t, 2> u32vec2_t;

typedef vec_t<float, 2> f32vec2_t;

typedef vec_t<double, 2> f64vec2_t;
typedef vec_t<double, 3> f64vec3_t;

typedef f64vec2_t vec2_t;
typedef f64vec3_t vec3_t;

#endif
