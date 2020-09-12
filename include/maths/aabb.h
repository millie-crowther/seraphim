#ifndef AABB_H
#define AABB_H

#include "matrix.h"

#include <limits>
#include <random>

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

    void capture_point(const vec_t<T, D> & c){
        min = vec::min(min, c);
        max = vec::max(max, c); 
    }
    
    vec_t<T, D> get_size() const {
        return max / 2 - min / 2;
    }

    vec_t<T, D> get_centre() const {
        return max / 2 + min / 2;
    }

    vec_t<T, D> get_vertex(uint8_t i) const {
        vec_t<T, D> v = min;
        for (uint8_t j = 0; j < D; j++){
            if (i & (1 << j)){
                v[i] = max[i];
            }
        }
        return v;
    }
    

    template<class TF>
    aabb_t<T, D> transform(const TF & tf){
        aabb_t<T, D> a;

        for (uint8_t i = 0; i < std::pow(2, D); i++){
            a.capture_point(tf(get_vertex(i)));
        } 

        return a;
    }

    aabb_t<T, D> operator&&(const aabb_t<T, D> & a) const {
        return aabb_t<T, D>(
            vec::max(min, a.min),
            vec::min(max, a.max)
        );
    }

    bool is_valid() const {
        return vec::all(min <= max);
    }
};

typedef aabb_t<double, 3> aabb3_t;

#endif
