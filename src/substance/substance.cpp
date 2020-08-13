#include "substance/substance.h"

#include <iostream>

substance_t::substance_t(std::shared_ptr<form_t> form, std::shared_ptr<matter_t> matter){
    this->form = form;
    this->matter = matter;
}

std::shared_ptr<form_t>
substance_t::get_form() const {
    return form;
}


std::shared_ptr<matter_t> 
substance_t::get_matter() const {
    return matter;
}
