#ifndef UUID_H
#define UUID_H

#include <cstdint>

class uuid_t {
private:
    uint8_t id[16];

public:
    uuid_t();

    bool operator==(const uuid_t& uuid) const;
};

#endif