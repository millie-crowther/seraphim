#include "core/seraphim.h"
#include "core/scheduler.h"
#include "maths/sdf/primitive.h"
#include "maths/sdf/platonic.h"
#include "physics/transform.h"

#include "core/random.h"

using namespace srph;

int main(){
    srph::seraphim_t engine;

    srph_material material;
    material.static_friction = 0.2;
    material.dynamic_friction = 0.1;
    material.density = 700.0;
    material.restitution = 0.3;

    srph_form form;

    vec3 floor_size;
    srph_vec3_fill(&floor_size, 100.0);
    srph_sdf * floor_sdf = srph_sdf_cuboid_create(&floor_size);
    material.colour = { 0.1, 0.8, 0.8 };
    srph_matter floor_matter(floor_sdf, &material, vec3_t(0.0, -100.0, 0.0), true);
    srph_create_substance(&engine, &form, &floor_matter);

/*
    srph_sdf_sphere sphere_sdf;
    srph_sdf_sphere_create(&sphere_sdf, 0.5);
    material.colour = { 0.8, 0.1, 0.8 };
    srph_matter sphere_matter(&sphere_sdf.sdf, &material, vec3_t(2.0, 3.0, 0.0), true);
    srph_create_substance(&engine, &form, &sphere_matter);
*/

    vec3 cube_size;
    srph_vec3_fill(&cube_size, 0.5);
    srph_sdf * cube_sdf = srph_sdf_cuboid_create(&cube_size);
    material.colour = { 0.8, 0.8, 0.1 };
    srph_matter cube1_matter(cube_sdf, &material, vec3_t(0.0, 3.0, 0.0), true);
    srph_create_substance(&engine, &form, &cube1_matter);

/*
    material.colour = { 0.4, 0.5, 0.1 };
    srph_matter cube2_matter(&cube_sdf.sdf, &material, vec3_t(-2.0, 3.1, 0.0), true);
    srph_create_substance(&engine, &form, &cube2_matter);
    
    srph_sdf_octahedron octa_sdf;
    srph_sdf_octahedron_create(&octa_sdf, 1.0);
    material.colour = { 0.9, 0.3, 0.1 };
    srph_matter octa_matter(&octa_sdf.sdf, &material, vec3_t(0.0, 3.0, 0.0), true);
    srph_create_substance(&engine, &form, &octa_matter);
*/

    engine.run();

    srph_sdf_destroy(floor_sdf);
    srph_sdf_destroy(cube_sdf);

    return 0;
}
