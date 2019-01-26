#ifndef BOUNDS_H
#define BOUNDS_H

#include "maths/vec.h"

class bounds_t {
private:
    // fields
    vec3_t min;
    vec3_t size;

public:
    // constructors
    bounds_t();
    bounds_t(const vec3_t& m, const vec3_t& s);

    // accessors
    vec3_t get_centre() const;
    vec3_t get_size() const;
    bounds_t get_octant(int i) const;
    int get_octant_id(const vec3_t& v) const;
    std::string to_string() const;    
    vec3_t get_vertex(int i) const;    

    // predicates
    bool contains(const vec3_t& v) const;
    bool contains(const bounds_t& b) const;

    // mutators
    void encapsulate_point(const vec3_t& v);
    void encapsulate_sphere(const vec3_t& v, float r);

    // factories
    static bounds_t max_bounds();
};

#endif
