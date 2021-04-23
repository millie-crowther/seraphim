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

namespace srph {
    template<class T, int M, int N>
    class matrix_t : protected std::array<T, M * N> {
    private:
        using super_t = std::array<T, M * N>; 

        template<int K, int P, typename... Xs>
        void construct(const matrix_t<T, P, 1> & x, Xs... xs){
            static_assert(K + P <= M * N, "Too much data in matrix constructor");
            static_assert(K + P == M * N || sizeof...(Xs) > 0, "Not enough data in matrix constructor");
           
            std::copy(x.begin(), x.end(), this->data() + K);
     
            if constexpr (sizeof...(Xs) != 0){
                construct<K + P>(xs...); 
            }
        }
     
        template<int K, typename... Xs>
        void construct(const T & x, Xs... xs){
            construct<K>(matrix_t<T, 1, 1>(x), xs...);
        }
        
    public:
        // constructors
        matrix_t() : matrix_t(T(0)){}

        matrix_t(const T & x){
            this->fill(x); 
        }
        
        template<class X, class... Xs>
        matrix_t(const X & x, Xs... xs){
            construct<0>(x, xs...);
        }
        
        // vector modifier operators
        void operator+=(const matrix_t<T, M, N> & x){
            std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::plus<T>());
        }

        void operator-=(const matrix_t<T, M, N> & x){
            std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::minus<T>());
        }

        void scale(const matrix_t<T, M, N> & x){
            std::transform(this->begin(), this->end(), x.begin(), this->begin(), std::multiplies<T>());
        }
        
        // scalar modifier operators 
        void operator*=(const T & x){
            scale(matrix_t<T, M, N>(x));    
        }

        // vector accessor operators  
        template<class S>
        matrix_t<decltype(T() + S()), M, N> operator+(const matrix_t<S, M, N> & x) const {
            matrix_t<decltype(T() + S()), M, N> r;
            std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::plus());
            return r;
        }

        template<class S>
        matrix_t<decltype(T() - S()), M, N> operator-(const matrix_t<S, M, N> & x) const {
            matrix_t<decltype(T() - S()), M, N> r;
            std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::minus());
            return r;
        } 

        template<class S>
        matrix_t<decltype(T() * S()), M, N> scaled(const matrix_t<S, M, N> & x) const {
            matrix_t<decltype(T() * S()), M, N> r;
            std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::multiplies());
            return r;
        }

        matrix_t<T, M, N> operator/(const matrix_t<T, M, N> & x) const {
            matrix_t<T, M, N> r;
            std::transform(this->begin(), this->end(), x.begin(), r.begin(), std::divides<T>());
            return r;
        }

        // scalar accessor operators
        matrix_t<T, M, N> operator-(const T & x) const {
            return *this - matrix_t<T, M, N>(x);
        }

        matrix_t<T, M, N> operator+(const T & x) const {
            return *this + matrix_t<T, M, N>(x);    
        }

        template<class S>
        matrix_t<decltype(T() * S()), M, N> operator*(const S & x) const {
            matrix_t<decltype(T() * S()), M, N> ms;
            for (int i = 0; i < M * N; i++){
                ms[i] = (*this)[i] * x;
            }
            return ms;
        }

        matrix_t<T, M, N> operator/(const T & x) const {
            return *this / matrix_t<T, M, N>(x);
        }

        // equality and ordering operators
        struct comparator_t {
            bool operator()(const matrix_t<T, M, N> & a, const matrix_t<T, M, N> & b){
                return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
            }
        };

        // getters
        T operator[](int i) const {
            return super_t::operator[](i);
        }

        T get(int row, int column) const {
            if (row >= M || column >= N){
                throw std::runtime_error("Error: Matrix index out of range.");
            }
            return (*this)[column * M + row];
        }

        // setters
        void set(int row, int column, const T & x){
            (*this)[column * M + row] = x;
        }

        T & operator[](int i){
            return super_t::operator[](i);
        }

        // iterators
        typename std::array<T, M * N>::iterator begin(){
            return super_t::begin();
        }

        typename std::array<T, M * N>::iterator end(){
            return super_t::end();
        }

        typename std::array<T, M * N>::const_iterator begin() const {
            return super_t::begin();
        }

        typename std::array<T, M * N>::const_iterator end() const {
            return super_t::end();
        }
    };

    template<class T, int N>
    using vec_t = matrix_t<T, N, 1>;

    typedef vec_t<uint32_t, 2> u32vec2_t;
    typedef vec_t<uint32_t, 3> u32vec3_t;

    typedef vec_t<float, 2> f32vec2_t;
    typedef vec_t<float, 3> f32vec3_t;
    typedef vec_t<float, 4> f32vec4_t;

    typedef vec_t<double, 2> f64vec2_t;
    typedef vec_t<double, 3> f64vec3_t;
    typedef vec_t<double, 4> f64vec4_t;

    typedef f64vec2_t vec2_t;
    typedef f64vec3_t vec3_t;
    typedef f64vec4_t vec4_t;

    namespace vec {
        template<class S, class T, int M, int N>
        decltype(S() * T()) dot(const matrix_t<S, M, N> & x, const matrix_t<T, M, N> & y){
            return std::transform_reduce(x.begin(), x.end(), y.begin(), decltype(S() * T())(0));
        }

        template<class T, int M, int N>
        T length(const matrix_t<T, M, N> & x){
            return std::sqrt(dot(x, x));
        }
    }

    namespace mat {
        template<class S, class T, int M, int N>
        matrix_t<S, M, N> cast(const matrix_t<T, M, N> & m){
            matrix_t<S, M, N> a;
            std::transform(m.begin(), m.end(), a.begin(), [](const T & x){
                return static_cast<S>(x);
            });
            return a;
        }
    }

    template<class T, int M, int N>
    matrix_t<T, M, N> operator/(const T & t, const matrix_t<T, M, N> & a){
        return matrix_t<T, M, N>(t) / a;
    }

    // equality operators
    template<class T, int N>
    bool operator==(const vec_t<T, N> & a, const vec_t<T, N> & b){
        if constexpr (std::is_floating_point<T>::value){
            return vec::length(a - b) < epsilon;
        } else {
            return std::equal(a.begin(), a.end(), b.begin());
        }
    }

    template<class T, int N>
    bool operator!=(const vec_t<T, N> & a, const vec_t<T, N> & b){
        return !(a == b);
    }

// output operators
    template<class T, int M, int N>
    std::ostream & operator<<(std::ostream & os, const matrix_t<T, M, N> & v){
        os << "mat" << M << 'x' << N << '(';
        for (int i = 0; i < M * N - 1; i++){
            os << v[i] << ", ";
        }
        os << v[M * N - 1] << ")";
        return os;
    } 
}

#endif
