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
    static const int max_iterations = 50;

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
  
    std::vector<collision_t> cs;

    for (uint8_t i = 0; i < 8; i++){
        vec3_t x = aabb.get_vertex(i);
        vec3_t dfdx_ = dfdx(x);
        double fx = f(x);
        
        for (int i = 0; i < max_iterations && fx > 0.0; i++){
            x -= dfdx_ * std::abs(fx);
            
            fx = f(x);
            dfdx_ = dfdx(x);
        }

        if (fx <= 0){
            cs.emplace_back(true, x, fx, a, b);
        }
    }

    if (cs.empty()){
        return collision_t::null();

    } else {
        vec3_t x;
        
        for (auto & c : cs){
            if (c.hit){
                x += c.x;
            }
        }

        x /= cs.size();

        return collision_t(true, x, f(x), a, b);
    }       
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

    // extraicate matters by rotation
    auto ra = a->get_offset_from_centre_of_mass(x);
    auto axis_a = vec::cross(ra, n);    

    auto rb = b->get_offset_from_centre_of_mass(x);
    auto axis_b = vec::cross(rb, n);

    // calculate collision impulse magnitude
    auto va = a->get_velocity(x);
    auto ia = mat::inverse(a->get_inertia_tensor());
    auto xa = vec::cross(ia * axis_a, ra); 
    auto ma = 1.0 / a->get_mass();
    auto mata = a->get_material(a->to_local_space(x));

    auto vb = b->get_velocity(x);
    auto ib = mat::inverse(b->get_inertia_tensor());
    auto xb = vec::cross(ib * axis_b, rb);
    auto mb = 1.0 / b->get_mass();
    auto matb = b->get_material(b->to_local_space(x));

    double CoR = std::max(mata.restitution, matb.restitution);
    double mu_s = std::max(mata.static_friction, matb.static_friction);
    double mu_d = std::max(mata.dynamic_friction, matb.dynamic_friction);
    auto vr = vb - va;
 
    double jr = 
        -(1.0 + CoR) * vec::dot(vr, n) /
        (ma + mb + vec::dot(xa + xb, n));
    
    // calculate frictional force
    vec3_t t  = vec::normalise(vr - vec::dot(vr, n) * n);
    double js = mu_s * jr;
    double jd = mu_d * jr;
    double vrt = vec::dot(vr, t);

    double vrta = a->get_mass() * vrt;
    vec3_t jfa = - (vrta <= js ? vrta : jd) * t;
    
    double vrtb = b->get_mass() * vrt;
    vec3_t jfb = - (vrtb <= js ? vrtb : jd) * t;

    jfa = vec3_t();
    jfb = vec3_t();

    // update velocities accordingly
    vec3_t ja  = -jr * n + jfa;
    vec3_t dva = ja / a->get_mass();
    vec3_t dwa = ia * vec::cross(ra, ja); 
    a->update_velocities(dva, dwa);

    vec3_t jb  =  jr * n + jfb;
    vec3_t dvb = jb / b->get_mass();
    vec3_t dwb = ib * vec::cross(rb, jb); 
    b->update_velocities(dvb, dwb);
}
