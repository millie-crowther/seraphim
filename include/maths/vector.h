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

void srph_vec3_fill(vec3 * v, double x);
void srph_vec3_set(vec3 * v, double x, double y, double z);

void srph_vec3_scale(vec3 * x, double s);
void srph_vec3_multiply(vec3 * x, const vec3 * a, const vec3 * b);
void srph_vec3_subtract(vec3 * x, const vec3 * a, const vec3 * b);

void srph_vec3_abs(vec3 * x);

void srph_vec3_normalise(vec3 * x);
double srph_vec3_dot(const vec3 * a, const vec3 * b);

double srph_vec2_length(const vec2 * x);
double srph_vec3_length(const vec3 * x);

#endif
