#include "core/seraphim.h"

#include "core/scheduler.h"
#include "maths/solver.h"
#include "substance/matter/sdf/primitive.h"

int main(){ 
    auto floor_substance = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<matter_t>(
            std::make_shared<primitive::cuboid_t>(vec3_t(100.0)), 
            material_t(vec3_t(0.1, 0.8, 0.8), 700.0),
            vec3_t(0.0, -100.0, 0.0)
        )
    );

    auto sphere = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<matter_t>(
            std::make_shared<primitive::sphere_t>(0.5), 
            material_t(vec3_t(0.8, 0.1, 0.8), 700.0),
            vec3_t(2.5, 3.0, 0.5)
        )
    );

    auto cube = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<matter_t>(
            std::make_shared<primitive::cuboid_t>(vec3_t(0.5)), 
            material_t(vec3_t(0.8, 0.8, 0.1), 700.0),
            vec3_t(-2.5, 3.0, 0.5)
        )
    );
    
    seraphim_t seraphim;

    seraphim.create(floor_substance);
    seraphim.create(sphere);
    seraphim.create(cube);

    std::shared_ptr<matter_t> m = floor_substance->get_matter();
    scheduler::schedule_every(hyper::iota, [m](){
        m->apply_force(vec3_t(0.0, 9.8, 0.0) * m->get_mass());
    });

    seraphim.run();

    return 0;
}
