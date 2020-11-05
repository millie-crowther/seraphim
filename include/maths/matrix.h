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

template<class T, int M, int N>
class matrix_t : protected std::array<T, M * N> {
private:
    using super_t = std::array<T, M * N>; 

    template<int K, int P, int Q, typename... Xs>
    void
    construct(const matrix_t<T, P, Q> & x, Xs... xs){
        static_assert(K + P * Q <= M * N, "Too much data in matrix constructor");
        static_assert(K + P * Q == M * N || sizeof...(Xs) > 0, "Not enough data in matrix constructor");
       
        std::copy(x.begin(), x.end(), this->data() + K);
 
        if constexpr (sizeof...(Xs) != 0){
            construct<K + P * Q>(xs...); 
        }
    }
 
    template<int K, typename... Xs>
    void
    construct(const T & x, Xs... xs){
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
        scale(matrix_t<T, M, N>(x));    
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
    scaled(const matrix_t<T, M, N> & x) const {
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

    matrix_t<T, M, 1>
    get_column(int c) const {
        matrix_t<T, M, 1> column;
        for (int row = 0; row < M; row++){
            column[row] = get(row, c);
        }
        return column;
    }

    matrix_t<T, N, 1>
    get_row(int r) const {
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
    static matrix_t<T, M, N>
    diagonal(const T & x){
        matrix_t<T, M, N> a;
        constexpr int size = std::min(M, N);
        for (int i = 0; i < size; i++){
            a.set(i, i, x);
        }
        return a;
    }
    
    static matrix_t<T, M, N>
    identity(){
        return diagonal(1);
    }
};

template<class T, int N>
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

typedef matrix_t<float, 3, 3> f32mat3_t;
typedef matrix_t<float, 4, 4> f32mat4_t;

typedef matrix_t<double, 3, 3> mat3_t;
using mat4_t = matrix_t<double, 4, 4>;

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

    template<class T, int N>
    T 
    dot(const matrix_t<T, N, 1> & x, const matrix_t<T, N, 1> & y){
        matrix_t<T, N, 1> h = x * y;
        return std::accumulate(h.begin(), h.end(), T(0));
    }
    
    template<class T, int N>
    T 
    length(const matrix_t<T, N, 1> & x){
        return std::sqrt(dot(x, x));
    }   

    template<class T, int N>
    matrix_t<T, N, 1> 
    normalise(const matrix_t<T, N, 1> & x){
        T l = length(x);
        return x / (l == T(0) ? T(1) : l);
    }

    template<class T, int N>
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

    template<class T, int N, class F>
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
    
    template<class T, int N>
    matrix_t<T, N, 1>
    clamp(const matrix_t<T, N, 1> & x, const matrix_t<T, N, 1> & low, const matrix_t<T, N, 1> & high){
        auto result = x;
        for (int i = 0; i < N; i++){
            result[i] = std::clamp(x[i], low[i], high[i]);
        }
        return result;
    }

    template<class T, int N>
    matrix_t<T, N, 1>
    clamp(const matrix_t<T, N, 1> & x, const T & low, const T & high){
        return clamp(x, matrix_t<T, N, 1>(low), matrix_t<T, N, 1>(high));
    }

    template<class T, int N>
    matrix_t<T, N, 1>
    min(const matrix_t<T, N, 1> & x, const matrix_t<T, N, 1> & y){
        matrix_t<T, N, 1> r;
        auto f = [](const T & a, const T & b){ return std::min<T>(a, b); };
        std::transform(x.begin(), x.end(), y.begin(), r.begin(), f);
        return r;
    }

    template<class T, int N>
    matrix_t<T, N, 1>
    max(const matrix_t<T, N, 1> & x, const matrix_t<T, N, 1> & y){
        matrix_t<T, N, 1> r;
        auto f = [](const T & a, const T & b){ return std::max<T>(a, b); };
        std::transform(x.begin(), x.end(), y.begin(), r.begin(), f);
        return r;
    }

    template<class T, int N>
    matrix_t<T, N, 1>
    max(const matrix_t<T, N, 1> & x, const T & y){
        return max(x, matrix_t<T, N, 1>(y));
    }
    
    template<class T, int N>
    matrix_t<T, N, 1>
    abs(const matrix_t<T, N, 1> & x){
        matrix_t<T, N, 1> r;
        std::transform(x.begin(), x.end(), r.begin(), [](const T & a){ return std::abs(a); });
        return r;
    }
        
    template<int N>
    bool all(const matrix_t<bool, N, 1> & x){
        for (auto b : x){
            if (!b) return false;
        }
        return true;
    }

    template<class T, int N>
    matrix_t<T, N, 1> sign(const matrix_t<T, N, 1> & x){
        matrix_t<T, N, 1> r;

        for (int i = 0; i < N; i++){
            if constexpr (std::is_unsigned<T>::value){
                r[i] = x[i] == 0 ? T(0) : T(1);
            } else {
                r[i] = x[i] > 0 ? 1 : (x[i] < 0 ? -1 : 0);
            }
        }

        return r;
    }
}

namespace mat {
    template<class T>
    T 
    determinant(const matrix_t<T, 3, 3> & a){
        return vec::dot(a.get_column(0), vec::cross(a.get_column(1), a.get_column(2)));
    }

    template<class T, int M, int N> 
    matrix_t<T, N, M>
    transpose(const matrix_t<T, M, N> & a){
        matrix_t<T, N, M> at;
    
        for (int row = 0; row < M; row++){
            for (int col = 0; col < N; col++){
                at.set(col, row, a.get(row, col));
            }
        }
        
        return at;
    }

    template<class T>
    matrix_t<T, 3, 3>
    inverse(const matrix_t<T, 3, 3> & a){
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

    template<class T, int X, int Y, int Z>
    matrix_t<T, X, Z>
    multiply(const matrix_t<T, X, Y> & a, const matrix_t<T, Y, Z> & b){
        matrix_t<T, X, Z> ab;
        
        for (int m = 0; m < X; m++){
            for (int n = 0; n < Z; n++){
                ab.set(m, n, vec::dot(a.get_row(m), b.get_column(n)));
            }
        }

        return ab; 
    }
    
    template<class S, class T, int M, int N>
    matrix_t<S, M, N>
    cast(const matrix_t<T, M, N> & m){
        matrix_t<S, M, N> a;
        std::transform(m.begin(), m.end(), a.begin(), [](const T & x){
            return static_cast<S>(x);
        });
        return a;
    }
}

// ordering operators
template<int N>
vec_t<bool, N> operator!(const vec_t<bool, N> & x){
    vec_t<bool, N> nx;
    std::transform(x.begin(), x.end(), nx.begin(), std::logical_not<bool>());
    return nx;
}

template<class T, int N>
vec_t<bool, N> operator<(const vec_t<T, N> & a, const vec_t<T, N> & b){
    vec_t<bool, N> x;
    std::transform(a.begin(), a.end(), b.begin(), x.begin(), std::less<T>());
    return x;
}

template<class T, int N>
vec_t<bool, N> operator<=(const vec_t<T, N> & a, const vec_t<T, N> & b){
    return !(b < a);
}

// multiplication operators
template<class T, int M, int N>
matrix_t<T, M, N>
operator*(const T & x, const matrix_t<T, M, N> & a){
    return a * x;
}

template<class T, int N>
matrix_t<T, N, 1>
operator*(const matrix_t<T, N, 1> & a, const matrix_t<T, N, 1> & b){
    return a.scaled(b);
}

template<class T, uint8_t N>
void
operator*=(const matrix_t<T, N, 1> & a, const matrix_t<T, N, 1> & b){
    a.scale(b);
}

template<class T, int X, int Y, int Z>
matrix_t<T, X, Z>
operator*(const matrix_t<T, X, Y> & a, const matrix_t<T, Y, Z> & b){
    return mat::multiply(a, b);
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
    
// output operators
template<class T, int M, int N>
std::ostream & 
operator<<(std::ostream & os, const matrix_t<T, M, N> & v){
    os << "mat" << M << 'x' << N << '(';
    for (int i = 0; i < M * N - 1; i++){
        os << v[i] << ", ";
    }
    os << v[M * N - 1] << ")";
    return os;
} 

#endif
