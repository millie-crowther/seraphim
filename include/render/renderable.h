#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "mesh.h"

class renderable_t {
private:
    bool visible;
    std::shared_ptr<mesh_t> mesh;
    
public:
    // constructors
    renderable_t(const std::shared_ptr<mesh_t>& mesh);

    // getters
    bool is_visible();
    std::shared_ptr<mesh_t> get_mesh();
};

#endif
