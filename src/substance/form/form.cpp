#include "substance/form/form.h"

form_t::form_t(uint32_t root, std::shared_ptr<sdf3_t> sdf){
    this->root = root;
    this->sdf = sdf;
}

void
form_t::create_aabb(){
    aabb = std::make_shared<aabb3_t>();
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

std::shared_ptr<aabb3_t> 
form_t::get_aabb(){
    if (aabb == nullptr){
        create_aabb();
    }

    return aabb;
}

std::shared_ptr<sdf3_t>
form_t::get_sdf() const {
    return sdf;
}

uint32_t
form_t::get_root() const {
    return root;
}