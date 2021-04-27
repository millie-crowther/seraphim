#include "core/seraphim.h"
#include "maths/sdf/primitive.h"
#include "maths/sdf/platonic.h"

#include "cJSON.h"
#include <stdio.h>
#include <assert.h>
#include <ui/file.h>

int main(){
    srph::seraphim_t engine;

    material_t material;
    material.static_friction = 0.2;
    material.dynamic_friction = 0.1;
    material.density = 700.0;
    material.restitution = 0.5;

    form_t form;

    vec3 floor_size = {{ 100.0, 100.0, 100.0}};
    sdf_t * floor_sdf = sdf_cuboid_initialise(&engine, &floor_size);
    material.colour = { {0.1, 0.8, 0.8} };
    vec3 position = { {0.0, -100.0, 0.0} };
    matter_t floor_matter;
    srph_matter_init(&floor_matter, floor_sdf, &material, &position, true, true);
    srph_create_substance(&engine, &form, &floor_matter);

    vec3 cube_size = {{ 0.5, 0.5, 0.5 }};
    sdf_t * cube_sdf = sdf_cuboid_initialise(&engine, &cube_size);
    material.colour = { {0.8, 0.8, 0.1} };
    position = { {0.0, 3.0, 0.0} };
    matter_t cube_matter;
    srph_matter_init(&cube_matter, cube_sdf, &material, &position, true, false);
    srph_create_substance(&engine, &form, &cube_matter);

//    sdf_t * sphere_sdf = sdf_sphere_create(0.5);
//    material.colour = { {0.8, 0.1, 0.8} };
//    position = { {2.0, 3.0, 0.0} };
//    matter_t sphere_matter;
//    srph_matter_init(&sphere_matter, sphere_sdf, &material, &position, true, false);
//    srph_create_substance(&engine, &form, &sphere_matter);
//
//    sdf_t * torus_sdf = sdf_torus_create(0.5, 0.2);
//    material.colour = { {0.6, 0.3, 0.85} };
//    position = { {-2.0, 3.0, 0.0} };
//    matter_t torus_matter;
//    srph_matter_init(&torus_matter, torus_sdf, &material, &position, true, false);
//    srph_create_substance(&engine, &form, &torus_matter);

    engine.run();

    srph_cleanup(&engine);

    srph_matter_destroy(&floor_matter);
    srph_matter_destroy(&cube_matter);

    sdf_destroy(floor_sdf);
    sdf_destroy(cube_sdf);

    return 0;
}
