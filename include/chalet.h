#ifndef CHALET_H
#define CHALET_H

#include "actor.h"

class chalet_t : public actor_t {
protected:
    void tick(float delta, const input_t& input) override;

public:
    chalet_t();
};

#endif