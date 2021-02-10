#include "maths/sdf/compose.h"

#include <math.h>

double srph_intersection_sdf::phi(const srph::vec3_t & x){
    double pa = _a->phi(x);
    double pb = _b->phi(x);
/*
    // exponential smoothing. warning: prone to overflow
    double res = exp2(-_k * pa) + exp2(-_k * pb);
    std::cout << "res = " << res<< std::endl;
    std::cout << "pa = " << pa << std::endl;
    std::cout << "pb = " << pb << std::endl;
    return -log2(res) / _k;
*/

    // cubic smoothing
    //double h = fmax(_k - abs(pa - pb), 0.0) / _k;
    //return fmin(pa, pb) - pow(h, 3) * _k / 6.0;

    // unsmoothed
    return fmax(pa, pb);
}

void srph_create_intersection_sdf(srph_intersection_sdf * i_sdf, srph::sdf3_t * a, srph::sdf3_t * b){
    if (i_sdf == NULL){
        return;
    }

    i_sdf->_a = a;
    i_sdf->_b = b;
}
