#include <cJSON.h>
#include <ui/file.h>
#include "core/seraphim.h"
#include "maths/sdf/primitive.h"
#include "maths/sdf/platonic.h"

#include <stdio.h>

int main() {
	const char *game_filepath = "/home/millie/seraphim_game/";
	const char *json_filename = "game.json";
	char filepath[100] = { 0 };
	sprintf(filepath, "%s%s", game_filepath, json_filename);

	cJSON *game_json = file_load_json(filepath);
	cJSON *title_json = cJSON_GetObjectItem(game_json, "title");
	char title_string[100] = "Seraphim";
	if (title_json != NULL && title_json->type == cJSON_String) {
		strcpy(title_string, title_json->valuestring);
	}
	cJSON_Delete(game_json);

	seraphim_t engine(title_string);


	form_t form;

    material_t floor_material;
    vec3 floor_colour = { {0.1, 0.8, 0.8}
    };
    material_create(&floor_material, &engine.num_materials, &floor_colour);
	vec3 floor_size = { {100.0, 100.0, 100.0}
	};
	sdf_t floor_sdf;
	sdf_cuboid_create(&engine.num_sdfs, &floor_sdf, &floor_size);
	vec3 position = { {0.0, -100.0, 0.0}
	};
	matter_t floor_matter;
	matter_create(&floor_matter, &floor_sdf, &floor_material, &position, true, true);
	srph_create_substance(&engine, &form, &floor_matter);

	vec3 cube_size = { {0.5, 0.5, 0.5}
	};

	material_t cube_material;
    vec3 cube_colour = { {0.8, 0.8, 0.1}
    };
	material_create(&cube_material, &engine.num_materials, &cube_colour);
	sdf_t cube_sdf;
	sdf_cuboid_create(&engine.num_sdfs, &cube_sdf, &cube_size);
	position = { {0.0, 3.0, 0.0}
	};
	matter_t cube_matter;
	matter_create(&cube_matter, &cube_sdf, &cube_material, &position, true, false);
	srph_create_substance(&engine, &form, &cube_matter);

//    sdf_t * sphere_sdf = sdf_sphere_create(0.5);
//    material.colour = { {0.8, 0.1, 0.8} };
//    position = { {2.0, 3.0, 0.0} };
//    matter_t sphere_matter;
//    matter_create(&sphere_matter, sphere_sdf, &material, &position, true, false);
//    srph_create_substance(&engine, &form, &sphere_matter);
//
//    sdf_t * torus_sdf = sdf_torus_create(0.5, 0.2);
//    material.colour = { {0.6, 0.3, 0.85} };
//    position = { {-2.0, 3.0, 0.0} };
//    matter_t torus_matter;
//    matter_create(&torus_matter, torus_sdf, &material, &position, true, false);
//    srph_create_substance(&engine, &form, &torus_matter);

	engine.run();

	srph_cleanup(&engine);

	matter_destroy(&floor_matter);
	matter_destroy(&cube_matter);

	return 0;
}
