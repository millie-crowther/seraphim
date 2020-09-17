#include "physics/collision.h"

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

bool
collision_t::comparator_t::operator()(const collision_t & a, const collision_t & b){
    if (a.hit != b.hit){
        return a.hit && !b.hit;
    } else {
        return a.t < b.t;
    }
}

collision_t
seraph::physics::collide(std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b){
    static const int max_iterations = 20;

    aabb3_t aabb = a->get_aabb() && b->get_aabb();
    if (!aabb.is_valid()){
        return collision_t::null();
    }

    // TODO: reuse variables

    // detect collision
    auto f = [a, b](const vec3_t & x){
        vec3_t x_a   = a->get_transform().to_local_space(x);
        double phi_a = a->get_sdf()->phi(x_a); 
        
        vec3_t x_b   = b->get_transform().to_local_space(x);
        double phi_b = b->get_sdf()->phi(x_b); 

        return std::max(phi_a, phi_b);
    };

    auto dfdx = [a, b](const vec3_t & x){
        transform_t ta    = a->get_transform();
        vec3_t      x_a   = ta.to_local_space(x);
        double      phi_a = a->get_sdf()->phi(x_a); 
        
        transform_t tb    = b->get_transform();
        vec3_t      x_b   = tb.to_local_space(x);
        double      phi_b = b->get_sdf()->phi(x_b); 
                
        if (phi_a > phi_b){
            return ta.get_rotation() * a->get_sdf()->normal(x_a); 
        } else {
            return tb.get_rotation() * b->get_sdf()->normal(x_b); 
        }
    }; 

    vec3_t x = aabb.get_centre();
    double s = vec::length(aabb.get_size());
    collision_t c = collision_t::null();
    bool found = true;

    for (double depth = constant::epsilon; depth < s && found; depth *= 2){ 
        found = false;

        for (int i = 0; i < max_iterations && !found; i++){
            double fx = f(x);

            if (fx < c.fx){
                c = collision_t(true, x, fx, a, b);
                found = true;
            }

            x -= dfdx(x) * (fx + depth);
        }
    }

    return c;
}

void
seraph::physics::collision_correct(const collision_t & collision){
    auto a = collision.a;
    auto b = collision.b;
    auto x = collision.x;
     
    auto x_a = a->get_transform().to_local_space(x);
    auto n = a->get_transform().get_rotation() * a->get_sdf()->normal(x_a);

    // extricate matters by translation
    double depth = std::abs(collision.fx);
    double sm = a->get_mass() + b->get_mass();
    a->get_transform().translate(-depth * n * b->get_mass() / sm);
    b->get_transform().translate( depth * n * a->get_mass() / sm);     

    // extricate matters by rotation
    vec3_t ra = a->get_offset_from_centre_of_mass(x);
    vec3_t axis_a = vec::normalise(vec::cross(ra, n));
    double theta_a = -depth / vec::length(ra) * b->get_mass() / sm;;
    if (ra != vec3_t(0.0) && ra != n){
        a->get_transform().rotate(quat_t::angle_axis(theta_a, axis_a));
    }
         
    vec3_t rb = b->get_offset_from_centre_of_mass(x);
    vec3_t axis_b = vec::normalise(vec::cross(rb, n));
    double theta_b = depth / vec::length(rb) * a->get_mass() / sm;
    if (rb != vec3_t(0.0) && rb != n){
        b->get_transform().rotate(quat_t::angle_axis(theta_b, axis_b));
    }

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
