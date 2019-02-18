#ifndef DOP_H
#define DOP_H

#include <array>

#include "sdf.h"

template<unsigned int K>
class dop_t {
private:
    std::array<float, K> d;

public:
    dop_t(const sdf_t & sdf){

    }

    bool intersects(const dop_t<K> & dop) const {
        // TODO
        return true; 
    }

    bool intersects_plane(const vec3_t & v, const vec3_t & n) const {
        // TODO
        return true;
    }
};

#endif