#ifndef MATHS_VECTOR_H
#define MATHS_VECTOR_H

template <unsigned int N, unsigned int M> class mat_t;

#include <cmath>
#include "maths.h"
#include <array>
#include "mat.h"
#include <iostream>
#include <algorithm>
#include "core/constant.h"
#include <initializer_list>

template<unsigned int N>
class vec_t {
protected:
    std::array<double, N> xs;

public:
    vec_t() : vec_t(0){}

    vec_t(double x){
        xs.fill(x);
    }

    vec_t(const std::initializer_list<double> & l) : xs(l) {}

    /*
        TODO:
        
        template<typename T, std::size_t N>
        void Do(const T(&list)[N]) { // Good!
            static_assert(N == 3, "Exactly three elements are required.");
        }

        void Test() { Do({1,2,3}); }
    */

    double dot(const vec_t<N>& o) const {
        double result = 0;
        for (int i = 0; i < N; i++){
            result += xs[i] * o.xs[i];
        }
        return result;
    }

    /*
       norms
    */
    double square_norm() const {
        return dot(*this);
    }

    double norm() const {
        return sqrt(square_norm());
    }   

    /*
       projections
    */
    vec_t<N> project_vector(const vec_t<N> & v) const {
        auto v_n = v.normalise();
        return dot(v_n) * v_n;
    }

    vec_t<N> project_plane(const vec_t<N> & n) const {
        return *this - project_vector(n);
    }

    vec_t<N> project_plane(const vec_t<N> & o, const vec_t<N> & n) const {
        return (*this - o).project_plane(n);
    }

    double angle(const vec_t<N> & v){
        return std::acos(
            dot(v) * 
            maths::inverse_square_root(square_norm()) * 
            maths::inverse_square_root(v.square_norm())
        );
    }

    vec_t<N> lerp(const vec_t<N> & v, double alpha){
        return *this * (1.0 - alpha) + v * alpha;
    }

    vec_t<N> hadamard(const vec_t<N> & o) const {
        auto result = *this;
        for (int i = 0; i < N; i++){
            result.xs[i] *= o.xs[i];
        }
        return result;
    }

    vec_t<N> normalise() const {
        return *this * maths::inverse_square_root(square_norm());
    }
 
    /*
       overloaded operators
    */
    double operator[](int i) const {
        return xs[i];
    }

    double& operator[](int i){
        return xs[i];
    }

    vec_t<N> operator+(const vec_t<N>& v) const {
        auto ys = xs;
        for (int i = 0; i < N; i++){
            ys[i] += v.xs[i];
        }
        return vec_t<N>(ys);
    }

    void operator+=(const vec_t<N>& v){
        xs = (*this + v).xs;
    }

    void operator*=(double scale){
        xs = (*this * scale).xs;
    }

    void operator/=(double scale){
        *this *= 1.0 / scale;
    }
  
    vec_t<N> operator-(const vec_t<N>& v) const {
         return (*this) + (-v);
    } 

    vec_t<N> operator-() const {
        auto result = *this;
        for (int i = 0; i < N; i++){
            result.xs[i] = -xs[i];
        }
        return result;
    }

    vec_t<N> operator*(double scale) const {
        auto ys = xs;
        for (int i = 0; i < N; i++){
            ys[i] *= scale;
        }
        return vec_t<N>(ys);
    }

    bool operator==(const vec_t<N>& v) const {
        return (*this - v).square_length() < constant::epsilon * constant::epsilon;
    }

    vec_t<N> operator/(double scale) const {
        return *this * (1.0f / scale);
    }
};

template <unsigned int N>
vec_t<N> 
operator*(double scale, const vec_t<N>& v){
    return v * scale;
}

typedef vec_t<2> vec2_t;
typedef vec_t<4> vec4_t;

class vec3_t : public vec_t<3> {
public:
    vec3_t();
    vec3_t(double x);
    vec3_t(const std::initializer_list<double> & l);

    vec3_t cross(const vec3_t & v) const;
    vec3_t operator%(const vec3_t & v) const;
};

#endif
