#include "physics/collision.h"

#include "maths/nelder_mead.h"

using namespace seraph::physics;

collision_t::collision_t(
    bool hit, const vec3_t & x, double fx,  
    std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b
){
    this->hit = hit;
    this->x = x;
    this->fx = fx;
    this->t = t;
    this->a = a;
    this->b = b;
}

collision_t collision_t::null(){
    return collision_t(false, vec3_t(), 0.0, nullptr, nullptr);
}

collision_t
seraph::physics::collide(std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b){
    aabb3_t aabb = a->get_aabb() && b->get_aabb();
    if (!aabb.is_valid()){
        return collision_t::null();
    }

    auto f = [a, b](const vec3_t & x){
        vec3_t x_a   = a->get_transform().to_local_space(x);
        double phi_a = a->get_sdf()->phi(x_a); 
        
        vec3_t x_b   = b->get_transform().to_local_space(x);
        double phi_b = b->get_sdf()->phi(x_b); 

        return std::max(phi_a, phi_b);
    };
    
    std::array<vec3_t, 4> xs = {
        aabb.get_vertex(0), aabb.get_vertex(3),
        aabb.get_vertex(5), aabb.get_vertex(6)
    };

    auto result = srph::nelder_mead::minimise(f, xs);
    return collision_t(result.fx <= 0, result.x, result.fx, a, b);
}

void
seraph::physics::collision_correct(const collision_t & collision){
    auto a = collision.a;
    auto b = collision.b;
    auto x = collision.x;
     
    auto x_a = a->get_transform().to_local_space(x);
    auto n = a->get_transform().get_rotation() * a->get_sdf()->normal(x_a);

    // extricate matters 
    double depth = std::max(constant::epsilon, std::abs(collision.fx));
    double sm = a->get_mass() + b->get_mass();
    a->get_transform().translate(-depth * n * b->get_mass() / sm);
    b->get_transform().translate( depth * n * a->get_mass() / sm);     

    // calculate collision impulse magnitude
    auto mata = a->get_material(a->to_local_space(x));
    auto matb = b->get_material(b->to_local_space(x));

    double CoR = std::max(mata.restitution, matb.restitution);
    auto vr = b->get_velocity(x) - a->get_velocity(x);
 
    double jr = -(1.0 + CoR) * vec::dot(vr, n) / (
        1.0 / a->get_mass() + a->get_inverse_angular_mass(x, n) +
        1.0 / b->get_mass() + b->get_inverse_angular_mass(x, n)
    );
    
    // TODO: calculate frictional force
   // double mu_s = std::max(mata.static_friction, matb.static_friction);
   // double mu_d = std::max(mata.dynamic_friction, matb.dynamic_friction);
    
    // update velocities accordingly
    a->update_velocity(-jr, x, n);
    b->update_velocity( jr, x, n);
}
