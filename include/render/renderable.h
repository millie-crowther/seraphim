#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "mesh.h"
#include "scene/transform.h"

class renderable_t {
private:
    bool visible;
    std::shared_ptr<mesh_t> mesh;
    transform_t transform;
    
public:
    // constructors
    renderable_t(transform_t& parent, const std::shared_ptr<mesh_t>& mesh);

    // getters
    bool is_visible() const;
    std::shared_ptr<mesh_t> get_mesh() const;
    mat4_t get_matrix() const;
};

#endif
