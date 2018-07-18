#ifndef DUAL_QUAT_H
#define DUAL_QUAT_H

#include "quat.h"
#include "mat.h"

class dual_quat_t {
private:
    quat_t real;
    quat_t dual;

public:
    dual_quat_t(const quat_t& r, const quat_t& d);
   
    mat4_t to_matrix();
    dual_quat_t compose(const dual_quat_t& dq);
   
    static dual_quat_t look_at(const vec3_t& from, const vec3_t& to, const vec3_t& up); 
};

#endif
