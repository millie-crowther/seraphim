#include "render/substance.h"

substance_t::substance_t(std::shared_ptr<sdf3_t> sdf){
    this->sdf = sdf;
}

std::weak_ptr<sdf3_t>
substance_t::get_sdf() const {
    return sdf;
}

substance_t::data_t
substance_t::get_data() const {
    return data;
}


