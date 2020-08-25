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

template<class T, uint8_t N>
class vec_t : public std::array<T, N> {
public:
    // constructors
    vec_t() : vec_t(T(0)){}

    vec_t(const T & x){
        this->fill(x); 
    }

    template<class... Xs>
    vec_t(typename std::enable_if<sizeof...(Xs)+1 == N, T>::type x, Xs... _xs) : std::array<T, N>({ x, _xs...}) {}

    template<class S>    
    vec_t(const vec_t<S, N> & x){
        std::transform(x.begin(), x.end(), this->begin(), [](const S & s){ return static_cast<T>(s); });
    }

    // vector modifier operators
    void operator+=(const vec_t<T, N> & x){
        std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::plus<T>());
    }

    void operator-=(const vec_t<T, N> & x){
        std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::minus<T>());
    }

    void operator*=(const vec_t<T, N> & x){
        std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::multiplies<T>());
    }

    void operator/=(const vec_t<T, N> & x){
        std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::divides<T>());
    }

    // scalar modifier operators 
    void operator+=(const T & x){
        *this += vec_t<T, N>(x);
    }

    void operator-=(const T & x){
        *this -= vec_t<T, N>(x);
    }

    void operator*=(const T & x){
        *this *= vec_t<T, N>(x);    
    }

    void operator/=(const T & x){
        *this /= vec_t<T, N>(x);
    }

    // vector accessor operators  
    vec_t<T, N> operator+(const vec_t<T, N> & x) const {
        vec_t<T, N> r;
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::plus<T>());
        return r;
    }

    vec_t<T, N> operator-(const vec_t<T, N> & x) const {
        vec_t<T, N> r;
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::minus<T>());
        return r;
    } 

    vec_t<T, N> operator*(const vec_t<T, N> & x) const {
        vec_t<T, N> r;
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::multiplies<T>());
        return r;
    }

    vec_t<T, N> operator/(const vec_t<T, N> & x) const {
        vec_t<T, N> r;
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::divides<T>());
        return r;
    }
    
    // scalar accessor operators
    vec_t<T, N> operator-(const T & x) const {
        return *this - vec_t<T, N>(x);
    }

    vec_t<T, N> operator+(const T & x) const {
        return *this + vec_t<T, N>(x);    
    }

    vec_t<T, N> operator*(const T & x) const {
        return *this * vec_t<T, N>(x);
    }

    vec_t<T, N> operator/(const T & x) const {
        return *this / vec_t<T, N>(x);
    }

    // negation operator
    vec_t<T, N> operator-() const {
        return *this * vec_t<T, N>(T(-1));
    }

    // equality and ordering operators
    bool operator<(const vec_t<T, N> & x) const {
        return std::lexicographical_compare(this->begin(), this->end(), x.begin(), x.end());
    }

    bool operator==(const vec_t<T, N> & x) const {
        return std::equal(this->begin(), this->end(), x.begin());
    }

    bool operator!=(const vec_t<T, N> & x) const {
        return !(x == *this);
    }
};

namespace vec {
    template<class T>
    vec_t<T, 3> right(){
        return vec_t<T, 3>(T(1), T(0), T(0));
    }
    
    template<class T>
    vec_t<T, 3> up(){
        return vec_t<T, 3>(T(0), T(1), T(0));
    }

    template<class T>
    vec_t<T, 3> forward(){
        return vec_t<T, 3>(T(0), T(0), T(1));
    }

    template<class T, uint8_t N>
    vec_t<T, N>
    clamp(const vec_t<T, N> & x, const vec_t<T, N> & low, const vec_t<T, N> & high){
        auto result = x;
        for (int i = 0; i < N; i++){
            result[i] = std::clamp(x[i], low[i], high[i]);
        }
        return result;
    }

    template<class T, uint8_t N>
    T 
    dot(const vec_t<T, N> & x, const vec_t<T, N> & y){
        vec_t<T, N> h = x * y;
        return std::accumulate(h.begin(), h.end(), T(0));
    }
    
    template<class T, uint8_t N>
    T 
    length(const vec_t<T, N> & x){
        return std::sqrt(dot(x, x));
    }   

    template<class T, uint8_t N>
    vec_t<T, N> 
    normalise(const vec_t<T, N> & x){
        T l = length(x);
        return x / (l == T(0) ? T(1) : l);
    }

    template<class T, uint8_t N>
    vec_t<T, N> 
    min(const vec_t<T, N> & x, const vec_t<T, N> & y){
        vec_t<T, N> r;
        auto f = [](const T & a, const T & b){ return std::min<T>(a, b); };
        std::transform(x.begin(), x.end(), y.begin(), r.begin(), f);
        return r;
    }

    template<class T, uint8_t N>
    vec_t<T, N> 
    max(const vec_t<T, N> & x, const vec_t<T, N> & y){
        vec_t<T, N> r;
        auto f = [](const T & a, const T & b){ return std::max<T>(a, b); };
        std::transform(x.begin(), x.end(), y.begin(), r.begin(), f);
        return r;
    }

    template<class T, uint8_t N>
    vec_t<T, N>
    max(const vec_t<T, N> & x, const T & y){
        vec_t<T, N> r;
        auto f = [&](const T & a){ return std::max<T>(a, y); };
        std::transform(x.begin(), x.end(), r.begin(), f);
        return r;
    }
    
    template<class T, uint8_t N>
    vec_t<T, N>
    abs(const vec_t<T, N> & x){
        vec_t<T, N> r;
        std::transform(x.begin(), x.end(), r.begin(), [](const T & a){ return std::abs(a); });
        return r;
    } 

    template<class T, uint8_t N>
    T 
    volume(const vec_t<T, N> & x){
        T product = std::accumulate(x.begin(), x.end(), T(1), std::multiplies<T>());
        if constexpr (std::is_unsigned<T>::value){
            return product;
        } else {
            return std::abs(product);
        }
    }

    template<class T>
    vec_t<T, 3>
    cross(const vec_t<T, 3> & x, const vec_t<T, 3> & y){
        return vec_t<T, 3>(
            x[1] * y[2] - x[2] * y[1],
            x[2] * y[0] - x[0] * y[2],
            x[0] * y[1] - x[1] * y[0]
        );
    }

    // factories
    template<class T, uint8_t N, class F>
    vec_t<T, N> grad(const F & f, const vec_t<T, N> & x){
        vec_t<T, N> r;
        for (uint8_t i = 0; i < N; i++){
            vec_t<T, N> axis;
            axis[i] = constant::epsilon;
            r[i] = (f(x + axis) - f(x - axis));
        }
        return r / (2 * constant::epsilon);
    }
}

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
