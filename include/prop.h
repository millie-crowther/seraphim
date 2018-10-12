#ifndef PROP_H
#define PROP_H

#include "transform.h"
#include "renderable.h"

class prop_t : public renderable_t {
private:
    std::shared_ptr<mesh_t> mesh;

protected:
    transform_t transform;

public:
    // constructors and destructors
    prop_t();

    // setters
    void set_mesh(const std::shared_ptr<mesh_t>& mesh);
 
    // overridden functions
    model_t get_model() override;
};

#endif
