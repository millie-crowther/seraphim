#include "scene/transform.h"

transform_t::transform_t(){
    master = nullptr;
}

std::weak_ptr<transform_t>
transform_t::create_servant(){
    std::shared_ptr<transform_t> servant = std::make_shared<transform_t>();
    servant->master = this;
    servants.push_back(servant);
    return servant;
}