#include "sdf.h"

#include <chrono>
#include <iostream>
#include "bounds.h"

#include "maths/constants.h"


sdf_t::sdf_t(std::function<float(const vec3_t&)> phi){
    this->phi = phi;
}

double
sdf_t::distance(const vec3_t& p) const {
    return phi(p);
}

double
sdf_t::operator()(const vec3_t& v) const {
    return distance(v);
}
 
vec3_t
sdf_t::normal(const vec3_t& p) const {
    return vec3_t({
        phi(p + vec3_t({ constants::epsilon, 0, 0 })) - phi(p - vec3_t({ constants::epsilon, 0, 0 })),      
        phi(p + vec3_t({ 0, constants::epsilon, 0 })) - phi(p - vec3_t({ 0, constants::epsilon, 0 })),      
        phi(p + vec3_t({ 0, 0, constants::epsilon })) - phi(p - vec3_t({ 0, 0, constants::epsilon }))
    }).normalise();
}

bounds_t
sdf_t::get_bounds() const {
    static float precision = 0.1f;
    static int x = 0;

    // TODO: improve performance
    static void (*helper)(const bounds_t&, bounds_t&, const sdf_t& sdf) = 
    [](const bounds_t& b, bounds_t& full, const sdf_t& sdf){
        float d = sdf.distance(b.get_centre());
        if (d < 0){ 
            full.encapsulate_sphere(b.get_centre(), -d); 
        }
        x++;
        if (
	    b.get_size().length() > precision && 
	    std::abs(d) < (b.get_size() / 2.0f).length() &&
	    !full.contains(b)
	){
            for (int i = 0; i < 8; i++){
                helper(b.get_octant(i), full, sdf);
            }
        }
    };

    bounds_t result;
    auto start = std::chrono::high_resolution_clock::now();
    helper(bounds_t::max_bounds(), result, *this);
    auto current = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
        current - start
    ).count();

    std::cout << time << " : " << x << std::endl;
    return result;
}
