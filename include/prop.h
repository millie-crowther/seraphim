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
    prop_t();

    // getters
    std::shared_ptr<mesh_t> get_mesh();

    // setters
    void set_mesh(const std::shared_ptr<mesh_t>& mesh);

    // accessors
    mat4_t get_model_matrix();
};

#endif