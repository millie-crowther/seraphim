#include "chalet.h"

chalet_t::chalet_t(const std::shared_ptr<mesh_t> & mesh) : renderable(transform, mesh){}

renderable_t 
chalet_t::get_renderable(){
    return renderable;
}
