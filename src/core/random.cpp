#include "core/random.h"

#include <math.h>

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)
uint32_t srph_random_u32(srph_random * r){
    uint64_t oldstate = r->_state;

    // Advance internal state
    r->_state = oldstate * 6364136223846793005ULL + r->_sequence;

    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void srph_random_seed(srph_random * r, uint64_t state, uint64_t sequence){
    if (r == NULL){
        return;
    }

    r->_state = state;
    r->_sequence = sequence | 1;
    srph_random_u32(r);
}

double srph_random_f64(srph_random * r){
    union {
        uint32_t u[2];
        double d;
    } u;
    
    do {
        u.u[0] = srph_random_u32(r);
        u.u[1] = srph_random_u32(r);
    } while (!isfinite(u.d));

    int exp;
    frexp(u.d, &exp);

    return ldexp(u.d, -exp - 1) + 0.5;
}
   
double srph_random_f64_range(srph_random * r, double l, double u){
    return srph_random_f64(r) * (u - l) + l;
} 
