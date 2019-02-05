#ifndef SDF_H
#define SDF_H

#include "vec.h"
#include <functional>

class bounds_t;

class sdf_t {
private:
    std::function<double(const vec3_t&)> phi;

    bool is_dynamic;

public:
    // constructors and destructors
    sdf_t();
    sdf_t(const std::function<double(const vec3_t&)>& phi); 

    // accessors
    vec3_t normal(const vec3_t& p) const;
    double operator()(const vec3_t& v) const;

    // setters
    void set_is_dynamic(bool is_dynamic);
 
    bounds_t get_bounds() const;
    double volume();

    bool intersects_plane(const vec3_t & v, const vec3_t & n) const;

    // factories
    sdf_t intersection(const sdf_t& sdf) const;
};

#endif
