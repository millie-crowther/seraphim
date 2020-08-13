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

    void capture_sphere(const vec_t<T, D> & c, const T & r){
        min = min.min(c - vec_t<T, D>(std::abs(r)));
        max = max.max(c + vec_t<T, D>(std::abs(r)));
    }

    vec_t<T, D> get_min() const {
        return min;
    }

    vec_t<T, D> get_max() const {
        return max;
    }

    vec_t<T, D> get_size() const {
        return max / 2 - min / 2;
    }

    vec_t<T, D> get_centre() const {
        return max / 2 + min / 2;
    }
};

typedef aabb_t<double, 3> aabb3_t;

#endif
