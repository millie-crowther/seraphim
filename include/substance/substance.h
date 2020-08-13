#ifndef SUBSTANCE_H
#define SUBSTANCE_H

#include <memory>

#include "scene/transform.h"
#include "form/form.h"
#include "matter/matter.h"

class substance_t {
public:

    substance_t(std::shared_ptr<form_t> form, std::shared_ptr<matter_t> matter);

    std::shared_ptr<form_t> get_form() const;
    std::shared_ptr<matter_t> get_matter() const;

private:
    std::shared_ptr<form_t> form;
    std::shared_ptr<matter_t> matter;
};

#endif
