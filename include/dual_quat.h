#ifndef DUAL_QUAT_H
#define DUAL_QUAT_H

#include "quat.h"
#include "mat.h"

class dual_quat_t {
private:
    quat_t real;
    quat_t dual;

    dual_quat_t(const quat_t& r, const quat_t& d);

public:
    // constructors
    dual_quat_t();
    dual_quat_t(const quat_t& q, const vec3_t& v);
    
    // accessors
    vec3_t get_translation() const;
    quat_t get_rotation() const;
    dual_quat_t conjugate() const;
    vec3_t transform(const vec3_t& v) const;
    mat4_t to_matrix() const;

    // operators
    dual_quat_t operator+(const dual_quat_t& dq) const;
    dual_quat_t operator*(const dual_quat_t& dq) const;
    vec3_t operator*(const vec3_t& v) const;

    // factories
    static dual_quat_t identity();
    static dual_quat_t look_at(const vec3_t& from, const vec3_t& to, const vec3_t& up); 
};

#endif
