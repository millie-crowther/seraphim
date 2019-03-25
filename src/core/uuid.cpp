#include "core/uuid.h"

#include <cstring>

std::mt19937 uuid_t::engine;
bool uuid_t::is_initialised = false;

uuid_t::uuid_t(){
    if (!is_initialised){
        initialise();
    }

    // TODO: (1) seed mersenne twister
    //       (2) use timestamp to reduce (admittedly extremely small) chance of collision
    for (int i = 0; i < 4; i++){
        id[i] = engine();
    }
}

void 
uuid_t::initialise() {
    is_initialised = true;
}

bool
uuid_t::operator==(const uuid_t & uuid) const {
    return std::memcmp(id, uuid.id, 16) == 0;
}


bool
uuid_t::comparator_t::operator()(const uuid_t & a, const uuid_t & b) const {
    for (int i = 0; i < 4; i++){
        if (a.id[i] != b.id[i]){
            return a.id[i] < b.id[i];
        }
    }

    return false;
}