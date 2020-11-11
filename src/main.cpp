#include "core/seraphim.h"

#include "core/scheduler.h"
#include "maths/sdf/primitive.h"
#include "maths/sdf/platonic.h"
#include "physics/transform.h"

int main(){
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
            vec3_t(-2.0, 1.5, 0),
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
  
    srph::seraphim_t seraphim;

    seraphim.create(floor_substance);
    seraphim.create(sphere);
    seraphim.create(torus);
    seraphim.create(cube);
    seraphim.create(octahedron);

    seraphim.run();

    return 0;
}
