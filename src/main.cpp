#include "core/seraphim.h"

#include "substance/matter/primitive.h"

int main(){ 
    // auto now = std::chrono::high_resolution_clock::now();
    // double theta = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() / 1000.0;
    // cube->get_matter()->set_rotation(quat_t::angle_axis(theta / 5.0, vec3_t::up()));
    // sphere->get_matter()->set_position(vec3_t(
    //     std::sin(theta) * 0.5,
    //     1.0, 
    //     std::cos(theta) * 0.5
    // ));

    auto floor_substance = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<matter_t>(
            std::make_shared<primitive::cuboid_t>(vec3_t(100.0)), 
            material_t(vec3_t(0.1, 0.8, 0.8), 700.0)
        )
    );
    floor_substance->get_matter()->set_position(vec3_t(0.0, -100.0, 0.0));

    auto sphere = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<matter_t>(
            std::make_shared<primitive::sphere_t>(0.5), 
            material_t(vec3_t(0.8, 0.1, 0.8), 700.0)
        )
    );

    auto cube = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<matter_t>(
            std::make_shared<primitive::cuboid_t>(vec3_t(0.5)), 
            material_t(vec3_t(0.8, 0.8, 0.1), 700.0)
        )
    );

    cube->get_matter()->set_position(vec3_t(-2.5, 1.0, 0.5));
    
    seraphim_t seraphim;

    seraphim.create(floor_substance);
    seraphim.create(sphere);
    seraphim.create(cube);

    seraphim.run();
}
