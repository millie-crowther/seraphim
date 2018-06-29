#include "sdf.h"

constexpr float sdf_t::epsilon;

sdf_t::sdf_t(std::function<float(glm::vec3)> phi){
    this->phi = phi;
}

float
sdf_t::operator()(glm::vec3 p){
    return phi(p);
}

glm::vec3
sdf_t::normal(glm::vec3 p){
    glm::vec3 n = glm::vec3(
        phi(p + glm::vec3(epsilon, 0, 0)) - phi(p - glm::vec3(epsilon, 0, 0)),      
        phi(p + glm::vec3(0, epsilon, 0)) - phi(p - glm::vec3(0, epsilon, 0)),      
        phi(p + glm::vec3(0, 0, epsilon)) - phi(p - glm::vec3(0, 0, epsilon))
    );

    return glm::length(n) > 0.0f ? glm::normalize(n) : n;
}
