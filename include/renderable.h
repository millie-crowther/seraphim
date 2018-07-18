#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "mesh.h"

struct model_t {
//    glm::mat4 tf;
    mesh_t * mesh;

    //model_t(glm::mat4 t, mesh_t * m){ tf = t; mesh = m; }
};

class renderable_t {
public:
    // virtual methods
    virtual bool is_visible(){ return true; }

    // abstract methods
    virtual model_t get_model() = 0;
};

#endif
