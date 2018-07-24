#include "sdf.h"

#include <chrono>
#include <iostream>

constexpr float sdf_t::epsilon;

sdf_t::sdf_t(std::function<float(const vec3_t&)> phi){
    this->phi = phi;
}

float
sdf_t::distance(const vec3_t& p) const {
    return phi(p);
}

vec3_t
sdf_t::normal(const vec3_t& p) const {
    return vec3_t({
        phi(p + vec3_t({ epsilon, 0, 0 })) - phi(p - vec3_t({ epsilon, 0, 0 })),      
        phi(p + vec3_t({ 0, epsilon, 0 })) - phi(p - vec3_t({ 0, epsilon, 0 })),      
        phi(p + vec3_t({ 0, 0, epsilon })) - phi(p - vec3_t({ 0, 0, epsilon }))
    }).normalise();
}

bounds_t
sdf_t::get_bounds(){
    static int x = 0;
    static void (*helper)(const bounds_t&, bounds_t&, const sdf_t& sdf) = 
    [](const bounds_t& b, bounds_t& full, const sdf_t& sdf){
        float d = sdf.distance(b.get_centre());
        x++;
        if (d < 0){ 
            full.encapsulate_sphere(b.get_centre(), -d); 
        }

        if (b.get_size().length() > 0.1f && std::abs(d) < (b.get_size() / 2.0f).length()){
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

    std::cout << time << std::endl;
    return result;
}
