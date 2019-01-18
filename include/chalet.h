#ifndef CHALET_H
#define CHALET_H

#include "render/renderable.h"

class chalet_t {
private:
    renderable_t renderable;
    transform_t transform;

public:
    chalet_t(const std::shared_ptr<mesh_t> & mesh);

    renderable_t get_renderable();
};

#endif
