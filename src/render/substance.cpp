#include "render/substance.h"

#include <iostream>

substance_t::substance_t(uint32_t root, uint32_t id, std::shared_ptr<sdf3_t> sdf){
    this->sdf = sdf;
    this->root = root;
    this->id = id;
}

std::weak_ptr<sdf3_t>
substance_t::get_sdf() const {
    return sdf;
}

substance_t::data_t
substance_t::get_data(){
    if (aabb == nullptr){
        create_aabb();
    }

    data_t data;

    data.id = id;
    data.root = root;
    data.x    = (aabb->get_max() + aabb->get_min()).cast<float>() / 2.0f;
    data.size = (aabb->get_max() - aabb->get_min()).cast<float>() / 2.0f;

    return data;
}

void
substance_t::create_aabb(){
    aabb = std::make_unique<aabb3_t>();
    aabb->capture_sphere(sdf->normal(vec3_t()) * -sdf->phi(vec3_t()), hyper::epsilon);

    bool has_touched_surface = true;

    const double precision = 32.0;

    for (uint32_t i = 0; i < 32 && has_touched_surface; i++){
        has_touched_surface = false;

        for (uint32_t face = 0; face < 6; face++){
            uint8_t ui = face % 3;
            uint8_t vi = (face + 1) % 3;
            uint8_t wi = (face + 2) % 3;

            vec3_t min = aabb->get_min() - hyper::epsilon;
            vec3_t max = aabb->get_max() + hyper::epsilon;

            vec3_t x;
            x[wi] = face < 3 ? min[wi] : max[wi];

            double du = std::max((max[ui] - min[ui]) / precision, hyper::epsilon);

            for (x[ui] = min[ui]; x[ui] < max[ui]; x[ui] += du){
                for (x[vi] = min[vi]; x[vi] < max[vi]; x[vi] += hyper::epsilon){
                    auto phi = sdf->phi(x);

                    if (phi < 0){
                        has_touched_surface = true;
                        aabb->capture_sphere(x, phi);
                    }

                    x[vi] += std::abs(phi);
                }
            }
        }   
    }
}

uint32_t 
substance_t::get_id() const {
    return id;
}