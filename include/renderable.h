#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "mesh.h"

struct model_t {
    mat4_t tf;
    std::shared_ptr<mesh_t> mesh;

    model_t(const mat4_t& t, const std::shared_ptr<mesh_t> m){ 
        tf = t; 
        mesh = m; 
    }
};

class renderable_t {
protected:
    // protected constructor for abstract class
    renderable_t();
    
public:
    // virtual methods
    virtual bool is_visible(){ return true; }

    // abstract methods
    virtual model_t get_model() = 0;
};

#endif
