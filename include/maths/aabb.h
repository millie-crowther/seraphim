#ifndef AABB_H
#define AABB_H

#include <limits>

#include "vec.h"

template<class T, uint8_t D>
class aabb_t {
private:
    vec_t<T, D> min;
    vec_t<T, D> max;

public:
    aabb_t() : aabb_t(
        vec_t<T, D>(std::numeric_limits<T>::max()), 
        vec_t<T, D>(std::numeric_limits<T>::min())
    ){}

    aabb_t(const vec_t<T, D> & min, const vec_t<T, D> & max){
        this->min = min;
        this->max = max;
    }

    void capture_point(const vec_t<T, D> & x){
        min = min.min(x);
        max = max.max(x);
    }

    void capture_sphere(const vec_t<T, D> & c, const T & r){
        min = min.min(c - vec_t<T, D>(std::abs(r)));
        max = max.max(c + vec_t<T, D>(std::abs(r)));
    }

    vec_t<T, D> subdivide(uint32_t i) const {
        vec_t<T, D> new_min = min;
        for (uint8_t j = 0; j < D; j++){
            if (i & (1 << j) != 0){
                new_min[j] = min[j] / 2 + max[j] / 2;
            }
        }
        return vec_t<T, D>(new_min, max);
    }

    vec_t<T, D> centre() const {
        return min / 2 + max / 2;
    }

    T diameter() const {
        return (max - min).norm();
    }
};

typedef aabb_t<double, 3> aabb3_t;

#endif