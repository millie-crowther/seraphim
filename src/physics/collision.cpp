#include "physics/collision.h"

#include "maths/optimise.h"

srph::collision_t::collision_t(
    bool hit, const vec3_t & x, double depth,  
    std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b
){
    this->hit = hit;
    this->x = x;
    this->depth = std::abs(depth);
    this->a = a;
    this->b = b;

    if (hit){
        x_a = a->to_local_space(x);
        x_b = b->to_local_space(x);
        n_a = a->get_rotation() * a->get_sdf()->normal(x_a);
        n_b = b->get_rotation() * b->get_sdf()->normal(x_b);
        
        auto ja = a->get_sdf()->jacobian(x_a);
        auto jb = b->get_sdf()->jacobian(x_b);

        n = vec::p_norm<1>(ja) <= vec::p_norm<1>(jb) ? n_a : -n_b;
    }
}

srph::collision_t
srph::collide(std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b){
    aabb3_t aabb = a->get_aabb() && b->get_aabb();
    if (!aabb.is_valid()){
        return srph::collision_t(false, vec3_t(), 0, nullptr, nullptr);
    }

    auto f = [a, b](const vec3_t & x){
        vec3_t x_a   = a->to_local_space(x);
        double phi_a = a->get_sdf()->phi(x_a); 
        
        vec3_t x_b   = b->to_local_space(x);
        double phi_b = b->get_sdf()->phi(x_b); 

        return  std::max(phi_a, phi_b);
    };

    auto f1 = [a, b](const vec3_t & x){
        vec3_t x_a   = a->to_local_space(x);
        double phi_a = a->get_sdf()->phi(x_a) + constant::epsilon; 
        
        vec3_t x_b   = b->to_local_space(x);
        double phi_b = b->get_sdf()->phi(x_b) + constant::epsilon; 

        return phi_a * phi_a + phi_b * phi_b;
    };
    
    std::array<vec3_t, 4> xs = {
        aabb.get_vertex(0), aabb.get_vertex(3),
        aabb.get_vertex(5), aabb.get_vertex(6)
    };

    auto result = srph::optimise::nelder_mead(f, xs);

    vec3_t x = result.fx < 0 ? srph::optimise::nelder_mead(f1, xs).x : result.x;

    return srph::collision_t(result.fx < 0, x, result.fx, a, b);
}

void
srph::collision_t::resting_correct(){
    std::cout << "resting contact" << std::endl;

    auto aa = a->get_acceleration(x);
    auto ab = b->get_acceleration(x);

    double ca = vec::dot(aa, n) - vec::dot(ab, n);    
    
    if (ca > 0){
        auto d = ca * n / 2;
        a->constrain_acceleration(-d); 
        b->constrain_acceleration( d); 
    }
}

void 
srph::colliding_contact_correct(const collision_t & c){
    // extricate matters 
    double sm = c.a->get_mass() + c.b->get_mass();
    c.a->translate(-c.depth * c.n_a * c.b->get_mass() / sm);
    c.b->translate(-c.depth * c.n_b * c.a->get_mass() / sm);
    
    auto vr = c.a->get_velocity(c.x) - c.b->get_velocity(c.x);

    // calculate collision impulse magnitude
    auto mata = c.a->get_material(c.x_a);
    auto matb = c.b->get_material(c.x_b);

    double CoR = std::max(mata.restitution, matb.restitution);

    double jr = (1.0 + CoR) * vec::dot(vr, c.n) / (
        1.0 / c.a->get_mass() + c.a->get_inverse_angular_mass(c.x, c.n) +
        1.0 / c.b->get_mass() + c.b->get_inverse_angular_mass(c.x, c.n)
    );

    // update velocities accordingly
    c.a->apply_impulse_at(-jr * c.n, c.x);
    c.b->apply_impulse_at( jr * c.n, c.x);
   
    std::vector<std::shared_ptr<matter_t>> ms = { c.a, c.b };
    auto fs = vec2_t(1.0, -1.0);

    for (int i = 0; i < 2; i++){
        auto m = ms[i];
        vec3_t v = vr * fs[i];

        auto x = m->to_local_space(c.x);
        auto n = m->get_rotation() * m->get_sdf()->normal(x);
        
        // calculate frictional force
        auto mat = m->get_material(m->to_local_space(c.x));
        double js = mat.static_friction * jr;
        double jd = mat.dynamic_friction * jr;
 
        vec3_t t = vec::normalise(v - vec::dot(v, n) * n);

        auto mvt = m->get_mass() * vec::dot(v, t);
        bool is_static = mvt <= js || std::abs(vec::dot(v, t)) < constant::epsilon;

        double k = is_static ? mvt : jd;
        m->apply_impulse_at(-k * t, c.x);
    }
}

void
srph::collision_t::correct(){
    auto vr = b->get_velocity(x) - a->get_velocity(x);
    auto vrn = vec::dot(vr, n);

    if (vrn < -constant::epsilon){
        colliding_contact_correct(*this);
    } else if (vrn < constant::epsilon){
        resting_correct();
    }
}
