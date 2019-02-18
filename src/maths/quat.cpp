#include "maths/quat.h"

quat_t::quat_t() : quat_t(1, 0, 0, 0){}

quat_t::quat_t(double w, double x, double y, double z) : quat_t(w, x, y, z, true) {}

quat_t::quat_t(double w, double x, double y, double z, bool should_normalise){
    this->w = w;
    this->x = x;
    this->y = y;
    this->z = z;

    if (should_normalise){
        normalise();
    }
}

void
quat_t::normalise(){
    double s = maths::inverse_square_root(w * w + x * x + y * y + z * z);
    
    if (s > 0){
        w *= s;
        x *= s;
        y *= s;
        z *= s;
    } else {
        w = 1;
        x = 0;
        y = 0;
        z = 0;
    }
}

quat_t
quat_t::look_at(const vec3_t & forward, const vec3_t & up){
    // TODO
    //     // your code from before
    // F = normalize(target - camera);   // lookAt
    // R = normalize(cross(F, worldUp)); // sideaxis
    // U = cross(R, F);                  // rotatedup

    // // note that R needed to be re-normalized
    // // since F and worldUp are not necessary perpendicular
    // // so must remove the sin(angle) factor of the cross-product
    // // same not true for U because dot(R, F) = 0

    // // adapted source
    // Quaternion q;
    // double trace = R.x + U.y + F.z;
    // if (trace > 0.0) {
    // double s = 0.5 / sqrt(trace + 1.0);
    // q.w = 0.25 / s;
    // q.x = (U.z - F.y) * s;
    // q.y = (F.x - R.z) * s;
    // q.z = (R.y - U.x) * s;
    // } else {
    // if (R.x > U.y && R.x > F.z) {
    //     double s = 2.0 * sqrt(1.0 + R.x - U.y - F.z);
    //     q.w = (U.z - F.y) / s;
    //     q.x = 0.25 * s;
    //     q.y = (U.x + R.y) / s;
    //     q.z = (F.x + R.z) / s;
    // } else if (U.y > F.z) {
    //     double s = 2.0 * sqrt(1.0 + U.y - R.x - F.z);
    //     q.w = (F.x - R.z) / s;
    //     q.x = (U.x + R.y) / s;
    //     q.y = 0.25 * s;
    //     q.z = (F.y + U.z) / s;
    // } else {
    //     double s = 2.0 * sqrt(1.0 + F.z - R.x - U.y);
    //     q.w = (R.y - U.x) / s;
    //     q.x = (F.x + R.z) / s;
    //     q.y = (F.y + U.z) / s;
    //     q.z = 0.25 * s;
    // }
    // }
    return quat_t();
}

quat_t
quat_t::angle_axis(double angle, const vec3_t & axis){
    vec3_t axis_n = axis.normalise();
    double s = std::sin(angle / 2);
    return quat_t(std::cos(angle / 2), axis_n[0] * s, axis_n[1] * s, axis_n[2] * s);
}

vec3_t
quat_t::rotate(const vec3_t & v) const {
    quat_t q = *this * quat_t(0, v[0], v[1], v[2], false) * inverse();
    return q.vector();
}

void
quat_t::operator*=(const quat_t & q){
    quat_t q1 = hamilton(q);
    w = q1.w;
    x = q1.x;
    y = q1.y;
    z = q1.z; 
}

quat_t
quat_t::operator*(const quat_t & q) const {
    return hamilton(q);
}

vec3_t
quat_t::operator*(const vec3_t & v) const {
    return rotate(v);
}

quat_t
quat_t::identity(){
    return quat_t();
}

quat_t
quat_t::inverse() const {
    return quat_t(w, -x, -y, -z);
}

vec3_t
quat_t::vector() const {
    return vec3_t(x, y, z);
}

double
quat_t::scalar() const {
    return w;
}

quat_t
quat_t::hamilton(const quat_t& q) const {
    // TODO
    return quat_t(
        // qs[0] * q.qs[1] + qs[1] * q.qs[0] + qs[2] * q.qs[3] - qs[3] * q.qs[2],
        // qs[0] * q.qs[2] - qs[1] * q.qs[3] + qs[2] * q.qs[0] + qs[3] * q.qs[1],
        // qs[0] * q.qs[3] + qs[1] * q.qs[2] - qs[2] * q.qs[1] + qs[3] * q.qs[0],
        // qs[0] * q.qs[0] - qs[1] * q.qs[1] - qs[2] * q.qs[2] - qs[3] * q.qs[3]
    );
}