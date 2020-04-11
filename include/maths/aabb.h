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
        min = min.min(c - vec_t<T, D>(r));
        max = max.max(c + vec_t<T, D>(r));
    }

    aabb_t<T, D> subdivide(uint32_t i) const {
        vec_t<T, D> new_min = min;
        for (uint8_t j = 0; j < D; j++){
            if (i & (1 << j) != 0){
                new_min[j] = min[j] / 2 + max[j] / 2;
            }
        }
        return aabb_t<T, D>(new_min, max);
    }

    vec_t<T, D> centre() const {
        return min / 2 + max / 2;
    }

    vec_t<T, D> get_size() const {
        return max - min;
    }

    bool contains_point(const vec_t<T, D> & x) const {
        for (uint8_t i = 0; i < D; i++){
            if (x[i] < min[i] || x[i] > max[i]){
                return false;
            }
        }

        return true;
    }

    bool contains_aabb(const aabb_t<T, D> & a) const {
        return contains_point(a.min) && contains_point(a.max);
    }
};

typedef aabb_t<double, 3> aabb3_t;

#endif