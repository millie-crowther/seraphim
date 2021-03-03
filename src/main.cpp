#include "core/seraphim.h"
#include "core/scheduler.h"
#include "maths/sdf/primitive.h"
#include "maths/sdf/platonic.h"
#include "physics/transform.h"

#include "core/random.h"

using namespace srph;

int main(){
    srph_material material;
    material.static_friction = 0.2;
    material.dynamic_friction = 0.1;
    material.density = 700.0;
    material.restitution = 0.3;

    srph_form form;

    srph_sdf_cuboid floor_sdf;
    vec3 floor_size;
    srph_vec3_fill(&floor_size, 100.0);
    srph_sdf_cuboid_create(&floor_sdf, &floor_size);
    material.colour = { 0.1, 0.8, 0.8 };
    srph_matter floor_matter(&floor_sdf.sdf, &material, vec3_t(0.0, -100.0, 0.0), true);
    auto floor_substance = std::make_shared<substance_t>(&form, &floor_matter);

    srph_sdf_sphere sphere_sdf;
    srph_sdf_sphere_create(&sphere_sdf, 0.5);
    material.colour = { 0.8, 0.1, 0.8 };
    srph_matter sphere_matter(&sphere_sdf.sdf, &material, vec3_t(2.0, 3.0, 0.0), true);
    auto sphere = std::make_shared<substance_t>(&form, &sphere_matter);

    srph_sdf_cuboid cube_sdf;
    vec3 cube_size;
    srph_vec3_fill(&cube_size, 0.5);
    srph_sdf_cuboid_create(&cube_sdf, &cube_size);
    material.colour = { 0.8, 0.8, 0.1 };
    srph_matter cube1_matter(&cube_sdf.sdf, &material, vec3_t(0.0, 3.0, 0.0), true);
    auto cube = std::make_shared<substance_t>(&form, &cube1_matter);

    material.colour = { 0.4, 0.5, 0.1 };
    srph_matter cube2_matter(&cube_sdf.sdf, &material, vec3_t(-2.0, 3.1, 0.0), true);
    auto cube2 = std::make_shared<substance_t>( &form, &cube2_matter);
    
    srph_sdf_octahedron octa_sdf;
    srph_sdf_octahedron_create(&octa_sdf, 1.0);
    material.colour = { 0.9, 0.3, 0.1 };
    srph_matter octa_matter(&octa_sdf.sdf, &material, vec3_t(0.0, 3.0, 0.0), true);
    auto octahedron = std::make_shared<substance_t>(&form, &octa_matter);

    srph::seraphim_t seraphim;

    seraphim.create(floor_substance);
//    seraphim.create(sphere);
    seraphim.create(cube);
  //  seraphim.create(cube2);
  //  seraphim.create(octahedron);

    seraphim.run();

    return 0;
}
