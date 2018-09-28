#ifndef PROP_H
#define PROP_H

#include "transform.h"
#include "mesh.h"

class prop_t {
private:
    std::shared_ptr<mesh_t> mesh;

protected:
    transform_t transform;

public:
    // constructors and destructors
    prop_t(const std::shared_ptr<mesh_t>& mesh);

    // getters
    std::shared_ptr<mesh_t> get_mesh();

    // accessors
    mat4_t get_model_matrix();
};

#endif