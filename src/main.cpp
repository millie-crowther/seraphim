#include "core/seraphim.h"

#include "core/scheduler.h"
#include "maths/sdf/primitive.h"
#include "maths/sdf/platonic.h"

#include "physics/transform.h"

#include "maths/nelder_mead.h"

double rosenbrock(const vec2_t & xy){
    return std::pow(1 - xy[0], 2) + 100 * std::pow(xy[1] - xy[0] * xy[0], 2); 
}

double easy(const vec2_t & xy){
    return vec::length(xy);
} 

int main(){ 
    std::array<vec2_t, 3> xs = {
        vec2_t(5, 5), vec2_t(5, -5),
        vec2_t(-5, 0)
    };
    
    auto result = srph::nelder_mead::minimise(easy, xs);
    std::cout << "hit  = " << std::boolalpha << result.hit << std::endl;
    std::cout << "x    = " << result.x  << std::endl;
    std::cout << "f(x) = " << result.fx << std::endl;

    return 0;

    auto floor_substance = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<matter_t>(
            std::make_shared<platonic::cube_t>(100.0), 
            material_t(vec3_t(0.1, 0.8, 0.8), 700.0, 0.3, 0.2, 0.1),
            vec3_t(0.0, -100.0, 0.0),
            true
        )
    );

    auto sphere = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<matter_t>(
            std::make_shared<primitive::sphere_t>(0.5), 
            material_t(vec3_t(0.8, 0.1, 0.8), 700.0, 0.5, 0.2, 0.1),
            vec3_t(2.0, 3.0, 0.0),
            true
        )
    );
        
    auto cube = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<matter_t>(
            std::make_shared<platonic::cube_t>(0.5), 
            material_t(vec3_t(0.8, 0.8, 0.1), 700.0, 0.2, 0.2, 0.1),
            vec3_t(-2.0, 3.0, 0),
            true
        )
    );

    auto octahedron = std::make_shared<substance_t>(
        std::make_shared<form_t>(),
        std::make_shared<matter_t>(
            std::make_shared<platonic::octahedron_t>(1.0),
            material_t(vec3_t(0.9, 0.3, 0.5), 700.0, 0.4, 0.2, 0.1),
            vec3_t(0.0, 3.0, 0.0),
            true
        )
    );
        
    auto torus = std::make_shared<substance_t>(
        std::make_shared<form_t>(),
        std::make_shared<matter_t>(
            std::make_shared<primitive::torus_t>(0.5, 0.25),
            material_t(vec3_t(0.3, 0.6, 0.8), 700.0, 0.4, 0.2, 0.1),
            vec3_t(-4.0, 3.0, 0.0),
            true
        )
    );
    
    seraphim_t seraphim;

    seraphim.create(floor_substance);
    seraphim.create(sphere);
    seraphim.create(torus);
    seraphim.create(cube);
    seraphim.create(octahedron);

    seraphim.run();

    return 0;
}
