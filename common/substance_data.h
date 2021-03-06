//
// Created by millie on 20/05/2021.
//

#ifndef SERAPHIM_SUBSTANCE_DATA_H
#define SERAPHIM_SUBSTANCE_DATA_H

typedef struct substance_data_t {
    float near;
    float far;
    uint32_t sdf_id;
    uint32_t material_id;

    vec3f r;
    uint32_t id;

    float transform[MAT4_SIZE];

    struct comparator_t {
        bool operator()(const substance_data_t &a, const substance_data_t &b) const;
    };
} substance_data_t;

static const substance_data_t null_substance_data = {
    .sdf_id = (uint32_t) ~0,
    .material_id = (uint32_t) ~0,
    .id = (uint32_t) ~0,
};

#endif //SERAPHIM_SUBSTANCE_DATA_H
