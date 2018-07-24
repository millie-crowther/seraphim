#ifndef MATHS_VECTOR_H
#define MATHS_VECTOR_H

template <unsigned int N, unsigned int M> class mat_t;

#include <cmath>
#include "maths.h"
#include <array>
#include "mat.h"
#include <iostream>

template <unsigned int N>
class vec_t {
protected:
    std::array<float, N> xs;

public:
    vec_t() : vec_t(0.0f){}

    vec_t(float x){
        xs.fill(x);
    }

    vec_t(const std::array<float, N>& xs){
        this->xs = xs;
    }

    float dot(const vec_t<N>& o) const {
        float result = 0;
        for (int i = 0; i < N; i++){
            result += xs[i] * o.xs[i];
        }
        return result;
    }

    std::string to_string() const {
        std::string r = "[";
	for (int i = 0; i < N - 1; i++){
            r += std::to_string(xs[i]) + ", ";
	}
	return r + std::to_string(xs[N-1]) + "]";
    }

    vec_t<3> cross(const vec_t<3>& v) const {
        return vec_t<3>({
            xs[1] * v.xs[2] - xs[2] * v.xs[1],
            xs[2] * v.xs[0] - xs[0] * v.xs[2],
            xs[0] * v.xs[1] - xs[1] * v.xs[0] 
        });
    }

    float square_length() const {
        return dot(*this);
    }

    float length() const {
        return sqrt(square_length());
    }   

    bool approx(const vec_t<N>& v){
	return maths::approx((*this - v).square_length(), 0);
    }

    vec_t<N> project_vector(const vec_t<N>& v) const {
        return dot(v.normalise()) * v.normalise();
    }

    vec_t<N> project_plane(const vec_t<N>& n) const {
        return *this - project_vector(n);
    }

    vec_t<N> project_plane(const vec_t<N>& o, const vec_t<N>& n) const {
        return (*this - o).project_plane(n);
    }

    float angle(const vec_t<N>& v){
        return std::acos(dot(v) / length() / v.length());
    }

    template <unsigned int M>
    mat_t<M, N> tensor(const vec_t<M>& o){
        mat_t<M, N> result;
        for (int i = 0; i < N; i++){
            for (int j = 0; j < M; j++){
                result[j][i] = xs[i] * o.xs[j];
            }
        }
        return result;
    }

    vec_t<N> hadamard(const vec_t<N>& o){
        vec_t<N> result = *this;
        for (int i = 0; i < N; i++){
            result.xs[i] *= o.xs[i];
        }
        return result;
    }

    vec_t<N> normalise() const {
        return (*this) / length();
    }
 
    /*
       overloaded operators
    */
    float operator[](int i) const {
        return xs[i];
    }

    float& operator[](int i){
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

    void operator*=(float scale){
        xs = (*this * scale).xs;
    }
  
    vec_t<N> operator-(const vec_t<N>& v) const {
         return (*this) + (-v);
    } 

    vec_t<N> operator-() const {
        vec_t<N> result = *this;
        for (int i = 0; i < N; i++){
            result.xs[i] = -xs[i];
        }
        return result;
    }

    vec_t<N> operator*(float scale) const {
        auto ys = xs;
        for (int i = 0; i < N; i++){
            ys[i] *= scale;
        }
        return vec_t<N>(ys);
    }

    vec_t<N> operator/(float scale) const {
        return *this * (1.0f / scale);
    }

    vec_t<3> operator%(const vec_t<3> v) const {
	return cross(v);
    }
};

typedef vec_t<2> vec2_t;
typedef vec_t<3> vec3_t;
typedef vec_t<4> vec4_t;

template <unsigned int N>
vec_t<N> 
operator*(float scale, const vec_t<N>& v){
    return v * scale;
}

#endif
