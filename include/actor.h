#ifndef ACTOR_H
#define ACTOR_H

#include "prop.h"
#include "effector.h"

class actor_t : public prop_t, public effector_t {
public:
    actor_t(const std::shared_ptr<mesh_t>& mesh);
};

#endif