#include "maths/sdf/mutate.h"

double srph_transform_sdf::phi(const srph::vec3_t & x){
    auto x1 = _transform->to_local_space(x);
    return _sdf->phi(x1);    
}

void srph_create_transform_sdf(srph_transform_sdf * transform_sdf, srph::transform_t * tf, srph::sdf3_t * sdf){
    if (transform_sdf == NULL){
        return;
    }

    transform_sdf->_transform = tf;
    transform_sdf->_sdf = sdf;
}
