#include "maths/bound.h"

#include <float.h>
#include <math.h>

#include <iostream>

void srph_bound3_create(srph_bound3 * b){
    if (b == NULL){
        return;
    }

    for (int i = 0; i < 3; i++){
        b->lower[i] =  DBL_MAX;
        b->upper[i] = -DBL_MAX;
    } 
}

void srph_bound3_intersection(const srph_bound3 * a, const srph_bound3 * b, srph_bound3 * intersect){
    if (a == NULL || b == NULL){
        srph_bound3_create(intersect);
        return;
    }

    for (int i = 0; i < 3; i++){
        intersect->lower[i] = fmax(a->lower[i], b->lower[i]);       
        intersect->upper[i] = fmin(a->upper[i], b->upper[i]);  
    }     
}

void srph_bound3_vertex(const srph_bound3 * b, int vertex_index, double * v){
    for (int i = 0; i < 3; i++){
        if ((vertex_index & (1 << i)) != 0){
            v[i] = b->upper[i];
        } else {
            v[i] = b->lower[i];
        }
    } 
}

bool srph_bound3_is_valid(const srph_bound3 * b){
    if (b == NULL){
        return false;
    }

    for (int i = 0; i < 3; i++){
        if (b->lower[i] > b->upper[i]){
            return false;
        }
    }

    return true;
}

void srph_bound3_midpoint(const srph_bound3 * b, double * v){
    for (int i = 0; i < 3; i++){
        v[i] = (b->upper[i] + b->lower[i]) / 2.0;
    }
}

void srph_bound3_radius(const srph_bound3 * b, double * v){
    for (int i = 0; i < 3; i++){
        v[i] = (b->upper[i] - b->lower[i]) / 2.0;
    }
}

void srph_bound3_capture(srph_bound3 * b, double * v){
    for (int i = 0; i < 3; i++){
        b->lower[i] = fmin(b->lower[i], v[i]);
        b->upper[i] = fmax(b->upper[i], v[i]);
    }
}

double srph_bound3_volume(const srph_bound3 * b){
    if (b == NULL){
        return 0.0;
    }

    double v = 1.0;
    for (int i = 0; i < 3; i++){
        v *= b->upper[i] - b->lower[i];
    }
    return v;
}
