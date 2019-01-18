#ifndef CHALET_H
#define CHALET_H

#include "render/renderable.h"

class chalet_t {
private:
    renderable_t renderable;

public:
    chalet_t(const std::shared_ptr<mesh_t>& mesh);
};

#endif
