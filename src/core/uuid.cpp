#include "core/uuid.h"

#include <cstring>

uuid_t::uuid_t(){
    // TODO: (1) seed mersenne twister
    //       (2) use timestamp to reduce (admittedly extremely small) chance of collision
    for (int i = 0; i < 4; i++){
        id[i] = engine();
    }
}

bool
uuid_t::operator==(const uuid_t & uuid) const {
    return std::memcmp(id, uuid.id, 16) == 0;
}