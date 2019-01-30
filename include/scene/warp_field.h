#ifndef WARP_FIELD_H
#define WARP_FIELD_H

#include <functional>
#include <vector>

#include "maths/vec.h"


class warp_field_t {
public:
    typedef std::function<vec3_t(const vec3_t&)> warp_func_t;

    warp_field_t();

    void add_warp_function(const warp_func_t& psi);

    void set_size(int size);
    void set_resolution(int resolution);

    void serialize(std::vector<vec3_t>& psi);

private:
    std::vector<warp_func_t> psis;
    int resolution;
    int size;

    static constexpr int default_resolution = 50;
    static constexpr int default_size = 1000;
};

#endif