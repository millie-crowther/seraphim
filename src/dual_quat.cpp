#include "dual_quat.h"

dual_quat_t::dual_quat_t(const quat_t& r, const quat_t& d){
    real = r;
    dual = d;
}

mat4_t
dual_quat_t::to_matrix(){
    mat3_t r = real.to_matrix();
    return mat4_t({
        r[0][0], r[1][0], r[2][0], dual[0], 
        r[0][1], r[1][1], r[2][1], dual[1],
        r[0][2], r[1][2], r[2][2], dual[2],
        0,       0,       0,       1 
    });
}

dual_quat_t
dual_quat_t::look_at(const vec3_t& from, const vec3_t& to, const vec3_t& up){
    quat_t r = quat_t::look_at(to - from, up);
    quat_t d = vec4_t({ up[0], up[1], up[2], 0 });

    return dual_quat_t(r, d);
}
