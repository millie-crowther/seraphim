#include "core/seraphim.h"
#include "maths/sdf/primitive.h"
#include "maths/sdf/platonic.h"

int main(){
    srph::seraphim_t engine;

    srph_material material;
    material.static_friction = 0.2;
    material.dynamic_friction = 0.1;
    material.density = 700.0;
    material.restitution = 0.5;

    srph_form form;

    vec3 floor_size = {{ 100.0, 100.0, 100.0}};
    srph_sdf * floor_sdf = srph_sdf_cuboid_create(&floor_size);
    material.colour = { {0.1, 0.8, 0.8} };
    vec3 position = { {0.0, -100.0, 0.0} };
    srph_matter floor_matter;
    srph_matter_init(&floor_matter, floor_sdf, &material, &position, true, true);
    srph_create_substance(&engine, &form, &floor_matter);

    vec3 cube_size = {{ 0.5, 0.5, 0.5 }};
    srph_sdf * cube_sdf = srph_sdf_cuboid_create(&cube_size);
    material.colour = { {0.8, 0.8, 0.1} };
    position = { {0.0, 3.0, 0.0} };
    srph_matter cube_matter;
    srph_matter_init(&cube_matter, cube_sdf, &material, &position, true, false);
    srph_create_substance(&engine, &form, &cube_matter);

//    srph_sdf * sphere_sdf = srph_sdf_sphere_create(0.5);
//    material.colour = { {0.8, 0.1, 0.8} };
//    position = { {2.0, 3.0, 0.0} };
//    srph_matter sphere_matter;
//    srph_matter_init(&sphere_matter, sphere_sdf, &material, &position, true, false);
//    srph_create_substance(&engine, &form, &sphere_matter);
//
//    srph_sdf * torus_sdf = srph_sdf_torus_create(0.5, 0.2);
//    material.colour = { {0.6, 0.3, 0.85} };
//    position = { {-2.0, 3.0, 0.0} };
//    srph_matter torus_matter;
//    srph_matter_init(&torus_matter, torus_sdf, &material, &position, true, false);
//    srph_create_substance(&engine, &form, &torus_matter);

    engine.run();

    srph_cleanup(&engine);

    srph_matter_destroy(&floor_matter);
    srph_matter_destroy(&cube_matter);

    srph_sdf_destroy(floor_sdf);
    srph_sdf_destroy(cube_sdf);

    return 0;
}
