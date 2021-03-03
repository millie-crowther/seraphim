#ifndef SERAPHIM_VECTOR_H
#define SERAPHIM_VECTOR_H

typedef struct vec3 {
    union {
        struct { 
            double x;
            double y;
            double z;
        };
        double raw[3];
    };
} vec3;

typedef struct vec2 {
    union {
        struct {
            double x;
            double y;
        };
        double raw[2];
    };
} vec2;

static const vec3 srph_vec3_zero    = { 0.0, 0.0, 0.0 };
static const vec3 srph_vec3_right   = { 1.0, 0.0, 0.0 };
static const vec3 srph_vec3_up      = { 0.0, 1.0, 0.0 };
static const vec3 srph_vec3_forward = { 0.0, 0.0, 1.0 };

void srph_vec3_fill(vec3 * v, double x);

// unary vector operations
void srph_vec3_abs(vec3 * abs_x, const vec3 * x);
void srph_vec3_scale(vec3 * sx, const vec3 * x, double s);
void srph_vec3_negate(vec3 * nx, const vec3 * x);

// binary vector operations
void srph_vec3_add(vec3 * x, const vec3 * a, const vec3 * b);
void srph_vec3_subtract(vec3 * x, const vec3 * a, const vec3 * b);
void srph_vec3_multiply(vec3 * x, const vec3 * a, const vec3 * b);
void srph_vec3_divide(vec3 * x, const vec3 * a, const vec3 * b);

void srph_vec3_project(vec3 * pa, const vec3 * a, const vec3 * b);
void srph_vec3_max_scalar(vec3 * max_x, const vec3 * x, double m);

double srph_vec3_distance(const vec3 * a, const vec3 * b);
void srph_vec3_normalise(vec3 * nx, const vec3 * x);
double srph_vec3_dot(const vec3 * a, const vec3 * b);
double srph_vec2_length(const vec2 * x);
double srph_vec3_length(const vec3 * x);

void srph_vec3_print(const vec3 * x);

#endif
