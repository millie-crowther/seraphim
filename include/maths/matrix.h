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

        matrix_t<T, M, 1> get_column(int c) const {
            matrix_t<T, M, 1> column;
            for (int row = 0; row < M; row++){
                column[row] = get(row, c);
            }
            return column;
        }

        matrix_t<T, N, 1> get_row(int r) const {
            matrix_t<T, N, 1> row;
            for (int column = 0; column < N; column++){
                row[column] = get(r, column);
            }
            return row;
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

        // factories
        static matrix_t<T, M, N> diagonal(const T & x){
            matrix_t<T, M, N> a;
            constexpr int size = std::min(M, N);
            for (int i = 0; i < size; i++){
                a.set(i, i, x);
            }
            return a;
        }
        
        static matrix_t<T, M, N> identity(){
            return diagonal(1);
        }
    };

    template<class T, int N>
    using vec_t = matrix_t<T, N, 1>;

    typedef vec_t<int32_t, 2> i32vec2_t;

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

    typedef matrix_t<float, 3, 3> f32mat3_t;
    typedef matrix_t<float, 4, 4> f32mat4_t;

    typedef matrix_t<double, 3, 3> mat3_t;
    using mat4_t = matrix_t<double, 4, 4>;

    namespace vec {
        template<class S, class T, int M, int N>
        decltype(S() * T()) dot(const matrix_t<S, M, N> & x, const matrix_t<T, M, N> & y){
            return std::transform_reduce(x.begin(), x.end(), y.begin(), decltype(S() * T())(0));
        }

        template<class T, int M, int N>
        T length(const matrix_t<T, M, N> & x){
            return std::sqrt(dot(x, x));
        }   

        template<class S, class T>
        vec_t<decltype(S() * T()), 3> cross(const vec_t<S, 3> & x, const vec_t<T, 3> & y){
            return vec_t<decltype(S()* T()), 3>(
                x[1] * y[2] - x[2] * y[1],
                x[2] * y[0] - x[0] * y[2],
                x[0] * y[1] - x[1] * y[0]
            );
        }
    }

    namespace mat {
        template<class T>
        T determinant(const matrix_t<T, 3, 3> & a){
            return vec::dot(a.get_column(0), vec::cross(a.get_column(1), a.get_column(2)));
        }

        template<class T, int M, int N> 
        matrix_t<T, N, M> transpose(const matrix_t<T, M, N> & a){
            matrix_t<T, N, M> at;
        
            for (int row = 0; row < M; row++){
                for (int col = 0; col < N; col++){
                    at.set(col, row, a.get(row, col));
                }
            }
            
            return at;
        }

        template<class T>
        matrix_t<T, 3, 3> inverse(const matrix_t<T, 3, 3> & a){
            matrix_t<T, 3, 3> a1(
                vec::cross(a.get_column(1), a.get_column(2)),
                vec::cross(a.get_column(2), a.get_column(0)),
                vec::cross(a.get_column(0), a.get_column(1))
            );

            T det = determinant(a);
            if (std::abs(det) < constant::epsilon){
                throw std::runtime_error("Error: tried to invert a singular matrix.");
            } else {
                return transpose(a1) / det;
            }
        } 

        template<class T, class S, int X, int Y, int Z>
        matrix_t<decltype(T() * S()), X, Z> multiply(const matrix_t<T, X, Y> & a, const matrix_t<S, Y, Z> & b){
            matrix_t<decltype(T() * S()), X, Z> ab;
            
            for (int m = 0; m < X; m++){
                for (int n = 0; n < Z; n++){
                    ab.set(m, n, vec::dot(a.get_row(m), b.get_column(n)));
                }
            }
            
            return ab; 
        }
        
        template<class S, class T, int M, int N>
        matrix_t<S, M, N> cast(const matrix_t<T, M, N> & m){
            matrix_t<S, M, N> a;
            std::transform(m.begin(), m.end(), a.begin(), [](const T & x){
                return static_cast<S>(x);
            });
            return a;
        }
    }

    template<class S, class T, int X, int Y, int Z>
    matrix_t<decltype(S() * T()), X, Z> operator*(const matrix_t<T, X, Y> & a, const matrix_t<S, Y, Z> & b){
        return mat::multiply(a, b);
    }

    template<class T, int M, int N>
    matrix_t<T, M, N> operator/(const T & t, const matrix_t<T, M, N> & a){
        return matrix_t<T, M, N>(t) / a;
    }

    // equality operators
    template<class T, int N>
    bool operator==(const vec_t<T, N> & a, const vec_t<T, N> & b){
        if constexpr (std::is_floating_point<T>::value){
            return vec::length(a - b) < constant::epsilon;
        } else {
            return std::equal(a.begin(), a.end(), b.begin());
        }
    }

    template<class T, int N>
    bool operator!=(const vec_t<T, N> & a, const vec_t<T, N> & b){
        return !(a == b);
    }
}

#endif
