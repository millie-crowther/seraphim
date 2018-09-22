#ifndef MATRIX_H
#define MATRIX_H

#include <string>
#include <array>
#include "vec.h"

template <unsigned int N> class vec_t;

template <unsigned int N, unsigned int M>
class mat_t {
private:
    //fields
    std::array<vec_t<M>, N> columns;

public:
    // constructors
    mat_t(){ 
        columns.fill(vec_t<M>());
    }

    mat_t(float f) : mat_t() {
        for (int i = 0; i < std::min(N, M); i++){
            columns[i][i] = f;
        }
    }

    mat_t(const std::array<vec_t<M>, N>& cs){
        columns = cs;
    }

    mat_t(const std::array<float, M * N>& xs){
        for (int i = 0; i < N * M; i++){
            columns[i % N][i/ M] = xs[i];
        }
    }

    mat_t<M, N> transpose() const {
        mat_t<M, N> result;
        for (int x = 0; x < N; x++){
            for (int y = 0; y < M; y++){
                result[y][x] = columns[x][y];
            }
        }
        return result;
    }

    std::string to_string(){
        std::string result = "";
        for (int y = 0; y < M; y++){
            result += "[";
            for (int x = 0; x < N; x++){
                result += std::to_string(columns[x][y]);
                if (x == N-1){
                    result += "]";
                    if (y < M-1){
                        result += "\n";
                    }
                } else {
                    result += ", ";
                }
            }
        }
        return result;
    }

    // operators
    vec_t<M> operator[](int i) const {
        return columns[i];
    }

    vec_t<M>& operator[](int i){
        return columns[i];
    }

    mat_t<N, M> operator+(const mat_t<N, M>& m){
        auto xs = columns;
        for (int i = 0; i < N; i++){
            xs[i] += m.columns[i];
        }
        return mat_t<N, M>(xs);
    }

    mat_t<N, M> operator*(float scale) const {
        auto result = *this;
        for (auto& c : result.columns){
            c *= scale;
        }
        return result;
    }

    vec_t<N> operator*(const vec_t<N>& v) const {
        vec_t<N> result;

        for (int i = 0; i < N; i++){
            for (int j = 0; j < M; j++){
                result[i] += columns[i][j] * v[i];
            }
        }
        
        return result;
    }

    // factories
    static mat_t<N, M> identity(){
        return mat_t<N, M>(1);
    }
};

// operatiors
template <unsigned int N, unsigned int M>
mat_t<N, M> operator*(float scale, const mat_t<N, M>& m){
    return m * scale;
}

// typedefs
typedef mat_t<2, 2> mat2_t;
typedef mat_t<3, 3> mat3_t;
typedef mat_t<4, 4> mat4_t;

typedef vec_t<3> vec3_t;

// factories
namespace matrix {
    mat3_t angle_axis(float angle, const vec3_t & axis);
    mat3_t look_at(const vec3_t & forward, const vec3_t& up);

    mat4_t perspective(float fov, float aspect, float near, float far);
    mat4_t look_at(const vec3_t & from, const vec3_t & to, const vec3_t & up);
}

#endif
