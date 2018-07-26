#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

#include "mesh.h"
#include "sdf.h"

class marching_cubes_t {
private:
    struct cell_t {
        float vs[8];
        vec3_t ps[8];
    };

    static const std::array<int, 256> edge_table;
    static const int tri_table[256][16];
    
    static vec3_t interpolate(
        float isolevel, const vec3_t& a, const vec3_t& b, 
        float alpha, float beta
    );

    static void create_mesh_for_cell(
        float isolevel, const cell_t& cell,
        std::vector<vertex_t>& vs, std::vector<unsigned int>& is
    );

public:
    static mesh_t polygonise(
        float resolution, float isolevel, VkCommandPool pool, 
        VkQueue queue, const sdf_t& sdf
    );
};

#endif