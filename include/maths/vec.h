#ifndef MATHS_VECTOR_H
#define MATHS_VECTOR_H

#include <cmath>
#include "maths.h"
#include <array>
#include <algorithm>
#include "core/constant.h"
#include <iostream>

template<class T, uint8_t N>
class vec_t {
protected:
    std::array<T, N> xs;

    vec_t(const std::array<T, N> & xs){
        this->xs = xs;
    }

public:
    /*
        constructors
    */
    vec_t() : vec_t(T(0)){}

    vec_t(const T & x){
        xs.fill(x); 
    }

    template<class... Xs>
    vec_t(typename std::enable_if<sizeof...(Xs)+1 == N, T>::type x, Xs... _xs) : xs({ x, _xs...}) {}

    /*
       norms
    */
    T square_norm() const {
        return *this * *this;
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
        vec_t<S, N> ys;
        for (uint8_t i = 0; i < N; i++){
            ys[i] = static_cast<S>(xs[i]);
        }
        return ys;
    }

    /*
        modifiers
    */ 
    template<class F>
    void for_each(const F & f){
        for (uint8_t i = 0; i < N; i++){ f(i); }
    }

    template<class F>
    void transform(const F & f){
        std::transform(xs.begin(), xs.end(), xs.begin(), f);
    }

    /* 
        accessors
    */
    template<class F>
    vec_t<T, N> enumerate(const F & f) const {
        std::array<T, N> x;
        for (uint8_t i = 0; i < N; i++){ x[i] = f(i); }
        return vec_t<T, N>(x);
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
    T operator[](uint8_t i) const {
        return xs[i];
    }

    T & operator[](uint8_t i){
        return xs[i];
    }

    /*
        modifier operators    
    */
    void operator+=(const vec_t<T, N> & v){
        for_each([&](uint8_t i){ xs[i] += v[i]; });
    }

    void operator-=(const vec_t<T, N> & v){
        for_each([&](uint8_t i){ xs[i] -= v[i]; });
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
        for (uint8_t i = 0; i < N; i++){ 
            result += xs[i] * x.xs[i]; 
        }
        return result;
    }

    vec_t<T, N> operator+(const vec_t<T, N> & x) const {
        return enumerate([&](uint8_t i){ return xs[i] + x.xs[i]; });
    }

    vec_t<T, N> operator-(const vec_t<T, N> & x) const {
        return enumerate([&](uint8_t i){ return xs[i] - x.xs[i]; });
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

    template<class S = vec_t<T, 3>>
    typename std::enable_if<N == 3, S>::type
    operator%(const vec_t<T, 3> & v) const {
        return vec_t<T, 3>(
            xs[1] * v.xs[2] - xs[2] * v.xs[1],
            xs[2] * v.xs[0] - xs[0] * v.xs[2],
            xs[0] * v.xs[1] - xs[1] * v.xs[0]
        );
    }

    /*
        factories
    */
    template<class F, class Tx, uint8_t Nx>
    static vec_t<T, N> nabla(const F & f, const vec_t<Tx, Nx> & x, const Tx & delta){
        // TODO: move division by (2 * delta) outside loop.
        //       for some reason, compiler gets confused if its outside loop
        return vec_t<T, N>().enumerate([&](uint8_t i){
            vec_t<Tx, Nx> axis;
            axis[i] = delta;
            return (f(x + axis) - f(x - axis)) / (2 * delta);
        });
    }
};

typedef vec_t<uint32_t, 2> u32vec2_t;

typedef vec_t<float, 2> f32vec2_t;
typedef vec_t<float, 3> f32vec3_t;
typedef vec_t<float, 4> f32vec4_t;

typedef vec_t<double, 2> f64vec2_t;
typedef vec_t<double, 3> f64vec3_t;
typedef vec_t<double, 4> f64vec4_t;

typedef f64vec2_t vec2_t;
typedef f64vec3_t vec3_t;
typedef f64vec4_t vec4_t;

#endif
