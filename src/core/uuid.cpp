#include "core/uuid.h"

#include <cstring>

uuid_t::uuid_t(){
    for (int i = 0; i < 4; i++){
        id[i] = engine();
    }
}

bool
uuid_t::operator==(const uuid_t & uuid) const {
    return std::memcmp(id, uuid.id, 16) == 0;
}