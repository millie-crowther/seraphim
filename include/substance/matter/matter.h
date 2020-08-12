#ifndef MATTER_H
#define MATTER_H

#include "substance/form/sdf.h"

#include <memory>

class matter_t {
public:
    matter_t(std::shared_ptr<sdf3_t> sdf, const vec3_t & colour);

    vec3_t get_colour(const vec3_t & x) const;

    std::shared_ptr<sdf3_t> get_sdf() const;

private:
    vec3_t colour;
    std::shared_ptr<sdf3_t> sdf;
};

#endif