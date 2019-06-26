#include "core/uuid.h"

#include <functional>
#include <algorithm>

bool uuid_t::is_seeded = false;
std::mt19937_64 uuid_t::engine;

uuid_t::uuid_t(){
    if (!is_seeded){
        is_seeded = true;
        seed();
    }

    id[0] = engine();
    id[1] = engine();
}

uuid_t::uuid_t(const uuid_t & uuid){
    id[0] = uuid.id[0];
    id[1] = uuid.id[1];
}

void
uuid_t::seed(){
    std::random_device::result_type random_data[std::mt19937_64::state_size];
    std::random_device source;
    std::generate(std::begin(random_data), std::end(random_data), std::ref(source));
    std::seed_seq seeds(std::begin(random_data), std::end(random_data));
    engine.seed(seeds);
}

bool 
uuid_t::operator<(const uuid_t & uuid) const {
    return id[0] < uuid.id[0] || (id[0] == uuid.id[0] && id[1] < uuid.id[1]);
}

void
uuid_t::operator=(const uuid_t & uuid){
    id[0] = uuid.id[0];
    id[1] = uuid.id[1];
}