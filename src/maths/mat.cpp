#include "maths/mat.h"

#include "maths/quat.h"

namespace matrix {
    mat3_t
    angle_axis(float angle, const vec3_t& axis){
        return quat_t::angle_axis(angle, axis).to_matrix();
    }

    mat3_t
    look_at(const vec3_t & forward, const vec3_t & up){
        vec3_t f = -forward.normalise();
        vec3_t s = (f % up).normalise();
        return mat3_t({ s, s % f, -f }).transpose();
    }

    mat4_t
    perspective(float fov, float aspect, float near, float far){
        float tan_f = std::tan(fov / 2.0f);

        return mat4_t({
            1.0f / (aspect * tan_f), 0, 0, 0,
            0, -1.0f / tan_f, 0, 0,
            0, 0, (far + near) / (near - far), 2 * far * near / (near - far),
            0, 0, -1, 0
        });
    }

    mat4_t
    look_at(
        const vec3_t & from, const vec3_t & to, const vec3_t & up
    ){
        vec3_t f = (to - from).normalise();
        vec3_t s = (f % up).normalise();
        vec3_t u = s % f;

        mat3_t m = look_at(f, up.normalise());

        return mat4_t({
            m[0][0], m[1][0], m[2][0], -s.dot(from),
            m[0][1], m[1][1], m[2][1], -u.dot(from),
            m[0][2], m[1][2], m[2][2], f.dot(from),
            0,       0,       0,       1
        });
    }
}