#ifndef FORM_H
#define FORM_H

#include <memory>

#include "sdf.h"

class form_t {
private:
    std::shared_ptr<sdf3_t> sdf;
    uint32_t root;
    std::shared_ptr<aabb3_t> aabb;

    void create_aabb();

public:
    form_t(uint32_t root, std::shared_ptr<sdf3_t> sdf);

    std::shared_ptr<aabb3_t> get_aabb();
    uint32_t get_root() const;
    std::shared_ptr<sdf3_t> get_sdf() const;

};

#endif