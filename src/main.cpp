#include "core/seraphim.h"
#include "maths/sdf/platonic.h"
#include "maths/sdf/primitive.h"
#include <cJSON.h>
#include <ui/file.h>

#include <stdio.h>
#include <cstring>

int main() {
    const char *game_filepath = "/home/millie/seraphim_game/";
    const char *json_filename = "game.json";
    char filepath[100] = {0};
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

    vec3 floor_colour = {{0.1, 0.8, 0.8}};
    material_t * floor_material = seraphim_create_material(&engine, &floor_colour);
    vec3 floor_size = {{100.0, 100.0, 100.0}};
    sdf_t * floor_sdf = seraphim_create_sdf(&engine, sdf_cuboid, &floor_size);
    vec3 position = {{0.0, -100.0, 0.0}};
    matter_t floor_matter;
    matter_create(&floor_matter, floor_sdf, floor_material, &position, true, true);
    seraphim_create_substance(&engine, &form, &floor_matter);

    vec3 cube_size = {{0.5, 0.5, 0.5}};

    vec3 cube_colour = {{0.8, 0.8, 0.1}};
    material_t * cube_material = seraphim_create_material(&engine, &cube_colour);
    sdf_t * cube_sdf = seraphim_create_sdf(&engine, sdf_cuboid, &cube_size);
    position = {{0.0, 3.0, 0.0}};
    matter_t cube_matter;
    matter_create(&cube_matter, cube_sdf, cube_material, &position, true, false);
    seraphim_create_substance(&engine, &form, &cube_matter);

    engine.run();

    seraphim_destroy(&engine);

    matter_destroy(&floor_matter);
    matter_destroy(&cube_matter);

    return 0;
}
