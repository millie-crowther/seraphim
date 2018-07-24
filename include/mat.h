#ifndef MATRIX_H
#define MATRIX_H

#include <string>
#include <array>
#include "vec.h"

template <unsigned int N> class vec_t;

template <unsigned int N, unsigned int M>
class mat_t {
protected:
    std::array<vec_t<M>, N> columns;

public:
    mat_t(){ 
        columns.fill(vec_t<M>());
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
                    result += "]\n";
                } else {
                    result += ", ";
                }
            }
        }
        return result;
    }

    /*
       overloaded operators
    */
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

    /*
       static methods
    */
    static mat_t<N, M> identity(){
        mat_t<N, M> result;
        for (int i = 0; i < std::min(N, M); i++){
            result[i][i] = 1.0f;
        }
        return result;
    }
};

typedef mat_t<2, 2> mat2_t;
typedef mat_t<3, 3> mat3_t;
typedef mat_t<4, 4> mat4_t;

template <unsigned int N, unsigned int M>
mat_t<N, M> operator*(float scale, const mat_t<N, M>& m){
    return m * scale;
}

#endif
