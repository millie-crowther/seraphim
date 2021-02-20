#ifndef SERAPHIM_BOUND
#define SERAPHIM_BOUND

typedef struct srph_bound3 {
    double lower[3];
    double upper[3];
} srph_bound3;

void srph_bound3_create(srph_bound3 * b);
void srph_bound3_intersection(const srph_bound3 * a, const srph_bound3 * b, srph_bound3 * i);
void srph_bound3_vertex(const srph_bound3 * b, int i, double * v);
void srph_bound3_midpoint(const srph_bound3 * b, double * v);
void srph_bound3_radius(const srph_bound3 * b, double * v);
void srph_bound3_capture(srph_bound3 * b, double * v);

bool srph_bound3_is_valid(const srph_bound3 * b);
double srph_bound3_volume(const srph_bound3 * b);

#endif
