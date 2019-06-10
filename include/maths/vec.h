#ifndef MATHS_VECTOR_H
#define MATHS_VECTOR_H

#include <cmath>
#include "maths.h"
#include <array>
#include <algorithm>
#include "core/constant.h"
#include <iostream>

template<class T, uint32_t N>
class vec_t {
protected:
    /*
        private fields
    */
    std::array<T, N> xs;

    /*
        private constructors
    */
    vec_t(const std::array<T, N> & _xs){
        xs = _xs;
    }

public:
    /*
        constructors
    */
    vec_t() : vec_t(0){}

    vec_t(const T & x){
        xs.fill(x); 
    }

    template<class... Xs>
    vec_t(typename std::enable_if<sizeof...(Xs)+1 == N, T>::type x, Xs... _xs) : xs({ x, _xs...}) {}

    /*
       norms
    */
    T square_norm() const {
        return operator*(*this);
    }

    T norm() const {
        return std::sqrt(square_norm());
    }   

    T chebyshev_norm() const {
        return std::accumulate(xs.begin(), xs.end(), 0, [](const T & a, const T & b){ 
            return std::max(a, std::abs(b));
        });
    }

    vec_t<T, N> normalise() const {
        T l = norm();
        if (l == T(0)){
            return vec_t<T, N>(0);
        }
        return *this / l;
    }

    /*
        accessors
    */
    template<class S>
    vec_t<S, N> cast() const {
        std::array<S, N> ys;
        for_each([&](uint32_t i){ ys[i] = static_cast<S>(xs[i]); });
        return vec_t<S, N>(ys);
    }

    /*
        modifiers
    */ 
    template<class F>
    void for_each(const F & f){
        for (uint32_t i = 0; i < N; i++){ f(i); }
    }

    template<class F>
    void transform(const F & f){
        std::transform(xs.begin(), xs.end(), xs.begin(), f);
    }

    template<class F>
    vec_t<T, N> map(const F & f) const {
        std::array<T, N> x;
        std::transform(xs.begin(), xs.end(), x.begin(), f);
        return vec_t<T, N>(x);
    }

    /*
        subscript operators
    */
    T operator[](uint32_t i) const {
        return xs[i];
    }

    T & operator[](uint32_t i){
        return xs[i];
    }

    /*
        modifier operators    
    */
    void operator+=(const vec_t<T, N> & v){
        for_each([&](uint32_t i){ xs[i] += v[i]; });
    }

    void operator-=(const vec_t<T, N> & v){
        for_each([&](uint32_t i){ xs[i] -= v[i]; });
    }

    void operator*=(const T & s){
        transform([&](const T & x){ return x * s; });
    }

    void operator/=(const T & s){
        transform([&](const T & x){ return x / s; });
    }
  
    /*
        accessor operators
    */
    T operator*(const vec_t<T, N> & x) const {
        T result = 0;
        for (uint32_t i = 0; i < N; i++){ 
            result += xs[i] * x.xs[i]; 
        }
        return result;
    }

    vec_t<T, N> operator+(const vec_t<T, N> & x) const {
        std::array<T, N> s;
        std::transform(xs.begin(), xs.end(), x.xs.begin(), s.begin(), std::plus<T>());
        return vec_t<T, N>(s);
    }

    vec_t<T, N> operator-(const vec_t<T, N> & x) const {
        return *this + -x;
    } 

    vec_t<T, N> operator-() const {
        return map(std::negate<T>());
    }

    vec_t<T, N> operator*(const T & s) const {
        return map([&](const T & x){ return x * s; });
    }

    vec_t<T, N> operator/(const T & s) const {
        return map([&](const T & x){ return x / s; });
    }

    bool operator==(const vec_t<T, N> & v) const {
        return (*this - v).square_norm() <= constant::epsilon * constant::epsilon;
    }

    template<class T1=vec_t<T, 3>>
    typename std::enable_if<N == 3, T1>::type
    operator%(const vec_t<T, 3> & v) const {
        return vec_t<T, 3>(
            xs[1] * v.xs[2] - xs[2] * v.xs[1],
            xs[2] * v.xs[0] - xs[0] * v.xs[2],
            xs[0] * v.xs[1] - xs[1] * v.xs[0]
        );
    }
};

typedef vec_t<uint32_t, 2> u32vec2_t;

typedef vec_t<float, 2> f32vec2_t;
typedef vec_t<float, 2> f32vec3_t;
typedef vec_t<float, 2> f32vec4_t;

typedef vec_t<double, 2> f64vec2_t;
typedef vec_t<double, 3> f64vec3_t;
typedef vec_t<double, 4> f64vec4_t;

typedef f64vec2_t vec2_t;
typedef f64vec3_t vec3_t;
typedef f64vec4_t vec4_t;

#endif
