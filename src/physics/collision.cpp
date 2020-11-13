#include "physics/collision.h"

#include "maths/nelder_mead.h"

srph::collision_t::collision_t(
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

srph::collision_t
srph::collide(std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b){
    aabb3_t aabb = a->get_aabb() && b->get_aabb();
    if (!aabb.is_valid()){
        return srph::collision_t(false, vec3_t(), 0, nullptr, nullptr);
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
    return srph::collision_t(result.fx <= 0, result.x, result.fx, a, b);
}

void
srph::collision_correct(const collision_t & c){
    auto x_a = c.a->get_transform().to_local_space(c.x);
    auto x_b = c.b->get_transform().to_local_space(c.x);
    auto n_a = c.a->get_transform().get_rotation() * c.a->get_sdf()->normal(x_a);
    auto n_b = c.b->get_transform().get_rotation() * c.b->get_sdf()->normal(x_b);

//*    
    n_a = vec3_t(0, 1, 0);
    n_b = vec3_t(0, -1, 0);
//*/

    // calculate collision impulse magnitude
    auto mata = c.a->get_material(c.a->to_local_space(c.x));
    auto matb = c.b->get_material(c.b->to_local_space(c.x));

    double CoR = std::max(mata.restitution, matb.restitution);
    auto vr = c.b->get_velocity(c.x) - c.a->get_velocity(c.x);
 
    auto n = vec::normalise(n_a - n_b);
    double jr = -(1.0 + CoR) * vec::dot(vr, n) / (
        1.0 / c.a->get_mass() + c.a->get_inverse_angular_mass(c.x, n) +
        1.0 / c.b->get_mass() + c.b->get_inverse_angular_mass(c.x, n)
    );
    
    // update velocities accordingly
    c.a->apply_impulse_at(-jr * n, c.x);
    c.b->apply_impulse_at( jr * n, c.x);
    
    double sm = c.a->get_mass() + c.b->get_mass();
    for (auto m : { c.a, c.b }){
        auto x = m->get_transform().to_local_space(c.x);
        auto n = m->get_transform().get_rotation() * m->get_sdf()->normal(x);
        
        // extricate matters 
        m->get_transform().translate(c.fx * n * (1 - m->get_mass() / sm));
        
        // calculate frictional force
    
        /*    
        auto mat = m->get_material(m->to_local_space(c.x));
        
        double js = mat.static_friction * jr;
        double jd = mat.dynamic_friction * jr;

        vec3_t fe = m->get_acceleration();
        vec3_t vrfe = vr;
        if (std::abs(vec::dot(vr, n)) < constant::epsilon){
            vrfe = fe;
        }

        vec3_t t;
        if (std::abs(vec::dot(vrfe, n)) > constant::epsilon){
            t = vec::normalise(vrfe - vec::dot(vrfe, n) * n);         
        }

        double k = jd;
        if (m->get_mass() * vec::dot(vr, t) <= js){
            k = m->get_mass() * vec::dot(vr, t);
        }

        m->apply_impulse_at(-k * t, c.x);
    //*/
    }
}
