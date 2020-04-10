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
    aabb_t(){
        min = vec_t<T, D>(std::numeric_limits<T>::max());
        max = vec_t<T, D>(std::numeric_limits<T>::min());
    }

    void capture(const vec_t<T, D> & x){
        min = min.min(x);
        max = max.max(x);
    }
};

typedef aabb_t<double, 3> aabb3_t;

#endif