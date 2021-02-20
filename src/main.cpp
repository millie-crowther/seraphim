#include "core/seraphim.h"
#include "core/scheduler.h"
#include "maths/sdf/primitive.h"
#include "maths/sdf/platonic.h"
#include "physics/transform.h"

#include "core/random.h"

using namespace srph;

int main(){
    double mu_s = 0.2;
    double mu_d = 0.1;

    srph_sdf_cuboid floor_sdf;
    vec3 floor_size;
    srph_vec3_fill(&floor_size, 100.0);
    srph_sdf_cuboid_create(&floor_sdf, &floor_size);

    auto floor_substance = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<srph_matter>(
            &floor_sdf.sdf, 
            material_t(vec3_t(0.1, 0.8, 0.8), 700.0, 0.3, mu_s, mu_d),
            vec3_t(0.0, -100.0, 0.0),
            true
        )
    );

    srph_sdf_sphere sphere_sdf;
    srph_sdf_sphere_create(&sphere_sdf, 0.5);

    auto sphere = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<srph_matter>(
            &sphere_sdf.sdf, 
            material_t(vec3_t(0.8, 0.1, 0.8), 700.0, 0.5, mu_s, mu_d),
            vec3_t(2.0, 3.0, 0.0),
            true
        )
    );

    srph_sdf_cuboid cube_sdf;
    vec3 cube_size;
    srph_vec3_fill(&cube_size, 0.5);
    srph_sdf_cuboid_create(&cube_sdf, &cube_size);

    auto cube = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<srph_matter>(
            &cube_sdf.sdf, 
            material_t(vec3_t(0.8, 0.8, 0.1), 700.0, 0.2, mu_s, mu_d),
            vec3_t(0, 3, 0),
            true
        )
    );

    auto cube2 = std::make_shared<substance_t>( 
        std::make_shared<form_t>(),
        std::make_shared<srph_matter>(
            &cube_sdf.sdf, 
            material_t(vec3_t(0.4, 0.5, 0.8), 700.0, 0.2, mu_s, mu_d),
            vec3_t(-2.0, 3.1, 0),
            true
        )
    );
    
    srph_sdf_octahedron octa_sdf;
    srph_sdf_octahedron_create(&octa_sdf, 1.0);

    auto octahedron = std::make_shared<substance_t>(
        std::make_shared<form_t>(),
        std::make_shared<srph_matter>(
            &octa_sdf.sdf,
            material_t(vec3_t(0.9, 0.3, 0.5), 700.0, 0.4, mu_s, mu_d),
            vec3_t(0.0, 3.0, 0.0),
            true
        )
    );
        
    srph_sdf_torus torus_sdf;
    srph_sdf_torus_create(&torus_sdf, 0.5, 0.25);


    auto torus = std::make_shared<substance_t>(
        std::make_shared<form_t>(),
        std::make_shared<srph_matter>(
            &torus_sdf.sdf,
            material_t(vec3_t(0.3, 0.6, 0.8), 700.0, 0.4, mu_s, mu_d),
            vec3_t(-4.0, 3.0, 0.0),
            true
        )
    );

    srph::seraphim_t seraphim;

    seraphim.create(floor_substance);
//    seraphim.create(sphere);
 //   seraphim.create(torus);
    seraphim.create(cube);
  //  seraphim.create(cube2);
  //  seraphim.create(octahedron);

    seraphim.run();

    return 0;
}
