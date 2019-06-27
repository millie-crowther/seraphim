#include "scene/transform.h"

transform_t transform_t::root_transform;

transform_t::transform_t(){
    master = nullptr;
}

std::weak_ptr<transform_t>
transform_t::create_servant(){
    std::shared_ptr<transform_t> servant;
    servant->master = this;
    servants.push_back(servant);
}