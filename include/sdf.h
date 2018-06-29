#ifndef SDF_H
#define SDF_H

#include <glm/glm.hpp>
#include <functional>

class sdf_t {
private:
    static constexpr float epsilon = 0.0001f;

    std::function<float(glm::vec3)> phi;

public:
    sdf_t(std::function<float(glm::vec3)> phi); 
    float operator()(glm::vec3 p);
    glm::vec3 normal(glm::vec3 p);
};

#endif
