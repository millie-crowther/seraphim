#ifndef PROP_H
#define PROP_H

#include "transform.h"
#include "mesh.h"

class prop_t {
protected:
    transform_t transform;
    mesh_t * mesh;

public:
    // constructors and destructors
    prop_t(mesh_t * mesh);
};

#endif