#ifndef UUID_H
#define UUID_H

#include <cstdint>
#include <random>

class uuid_t {
private:
    static std::mt19937 engine;

    // 128-bit number
    uint32_t id[4];

public:
    uuid_t();

    bool operator==(const uuid_t & uuid) const;
};

#endif