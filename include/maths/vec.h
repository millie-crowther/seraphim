#ifndef MATHS_VECTOR_H
#define MATHS_VECTOR_H

#include <cmath>
#include <array>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <type_traits>
#include <functional>

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
        std::transform(x.begin(), x.end(), this->begin(), [](const S & s){ return T(s); });
    }

    // norms
    T square_norm() const {
        return *this * *this;
    }

    T norm() const {
        return std::sqrt(square_norm());
    }   

    T chebyshev_norm() const {
        auto f = [](const T & a, const T & b){ return std::max(std::abs(a), std::abs(b)); };
        return std::accumulate(this->begin(), this->end(), T(0), f);
    }

    T manhattan_norm() const {
        auto f = [](const T & a, const T & b){ return std::abs(a) + std::abs(b); };
        return std::accumulate(this->begin(), this->end(), T(0), f);
    }

    vec_t<T, N> normalise() const {
        T l = norm();
        return *this / (l == T(0) ? T(1) : l);
    }

    // modifier operators   
    template<class S> 
    void operator+=(const S & x){
        std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::plus<T>());
    }

    template<class S>
    void operator-=(const S & x){
        std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::minus<T>());
    }

    void operator*=(const T & s){
        std::transform(this->begin(), this->end(), this->begin(), std::bind1st(std::multiplies<T>(), s));
    }

    void operator/=(const T & s){
        std::transform(this->begin(), this->end(), this->begin(), std::bind2nd(std::divides<T>(), s));
    }
  
    // accessors
    T operator*(const vec_t<T, N> & x) const {
        vec_t<T, N> h = hadamard(x);
        return std::accumulate(h.begin(), h.end(), T(0));
    }

    vec_t<T, N> operator+(const vec_t<T, N> & x) const {
        vec_t<T, N> r;
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::plus<T>());
        return r;
    }

    template<class S>
    typename std::enable_if<std::is_constructible<T, S>::value, vec_t<T, N>>::type
    operator-(const S & x) const {
        vec_t<T, N> r;
        std::transform(this->begin(), this->end(), r.begin(), std::bind2nd(std::minus<T>(), static_cast<T>(x)));
        return r;
    }

    template<class S>
    typename std::enable_if<std::is_constructible<T, S>::value, vec_t<T, N>>::type
    operator-(const vec_t<S, N> & x) const {
        vec_t<T, N> r;
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::minus<T>());
        return r;
    } 

    vec_t<T, N> operator-() const {
        vec_t<T, N> x;
        std::transform(this->begin(), this->end(), x.begin(), std::negate<T>());
        return x;
    }

    vec_t<T, N> operator+(const T & s) const {
        vec_t<T, N> x;
        std::transform(this->begin(), this->end(), x.begin(), std::bind1st(std::plus<T>(), s));
        return x;
    }

    vec_t<T, N> operator*(const T & s) const {
        vec_t<T, N> x;
        std::transform(this->begin(), this->end(), x.begin(), std::bind1st(std::multiplies<T>(), s));
        return x;
    }

    vec_t<T, N> operator/(const T & s) const {
        vec_t<T, N> x;
        std::transform(this->begin(), this->end(), x.begin(), std::bind2nd(std::divides<T>(), s));
        return x;
    }

    vec_t<T, N> hadamard(const vec_t<T, N> & x) const {
        vec_t<T, N> r;
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::multiplies<T>());
        return r;
    }

    vec_t<T, N> min(const vec_t<T, N> & x) const {
        vec_t<T, N> r;
        auto f = [](const T & a, const T & b){ return std::min<T>(a, b); };
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), f);
        return r;
    }

    vec_t<T, N> max(const vec_t<T, N> & x) const {
        vec_t<T, N> r;
        auto f = [](const T & a, const T & b){ return std::max<T>(a, b); };
        std::transform(this->begin(), this->end(), x.begin(), r.begin(), f);
        return r;
    }

    template<class S = vec_t<T, N>>
    typename std::enable_if<!std::is_unsigned<T>::value, S>::type 
    abs() const {
        vec_t<T, N> r;
        std::transform(this->begin(), this->end(), r.begin(), [](const T & a){ return std::abs(a); });
        return r;
    } 

    T volume() const {
        if constexpr (std::is_unsigned<T>::value){
            return std::accumulate(this->begin(), this->end(), T(1), std::multiplies<T>());
        } else {
            return std::abs(std::accumulate(this->begin(), this->end(), T(1), std::multiplies<T>()));
        }
    }

    template<class S>
    vec_t<S, N> cast() const {
        vec_t<S, N> x;
        std::transform(this->begin(), this->end(), x.begin(), [](const T & t){ return static_cast<S>(t); }); 
        return x;
    }

    template<class S = vec_t<T, 3>>
    typename std::enable_if<N == 3, S>::type
    operator%(const vec_t<T, 3> & v) const {
        return vec_t<T, 3>(
            (*this)[1] * v[2] - (*this)[2] * v[1],
            (*this)[2] * v[0] - (*this)[0] * v[2],
            (*this)[0] * v[1] - (*this)[1] * v[0]
        );
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

    // factories
    template<class F, class Tx, uint8_t Nx>
    static vec_t<T, N> nabla(const F & f, const vec_t<Tx, Nx> & x, const Tx & delta){
        vec_t<T, N> r;
        for (uint8_t i = 0; i < N; i++){
            vec_t<Tx, Nx> axis;
            axis[i] = delta;
            r[i] = (f(x + axis) - f(x - axis));
        }
        return r / (2 * delta);
    }

    template<uint8_t A>
    static typename std::enable_if<A < N, vec_t<T, N>>::type axis(){
        vec_t<T, N> result;
        result[A] = T(1);
        return result;
    }

    static vec_t<T, N> right(){
        return axis<0>();
    }

    static vec_t<T, N> up(){
        return axis<1>();
    }

    static vec_t<T, N> forward(){
        return axis<2>();
    }
};

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

#endif
