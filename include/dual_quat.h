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
    
};

#endif
