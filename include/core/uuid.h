#ifndef UUID_H
#define UUID_H

#include <cstdint>
#include <random>

class uuid_t {
private:
    // static fields
    static bool is_seeded;
    static std::mt19937_64 engine;

    // fields
    uint64_t id[2];

    // private static functions
    static void seed();

public:
    // constructors
    uuid_t();
    uuid_t(const uuid_t & uuid);

    // overloaded operators
    bool operator<(const uuid_t & uuid) const;
    void operator=(const uuid_t & uuid);
};

#endif