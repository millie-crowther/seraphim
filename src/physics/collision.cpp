#include "physics/collision.h"

#include "maths/optimise.h"

srph::collision_t::collision_t(
    std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b
){
    this->a = a;
    this->b = b;
    
    auto f = [a, b](const vec3_t & x){
        vec3_t x_a   = a->to_local_space(x);
        double phi_a = a->get_sdf()->phi(x_a); 
        
        vec3_t x_b   = b->to_local_space(x);
        double phi_b = b->get_sdf()->phi(x_b); 

        return std::max(phi_a, phi_b);
    };
    
    aabb3_t aabb = a->get_aabb() && b->get_aabb();
    if (!aabb.is_valid()){
        hit = false;
    } else {
        std::array<vec3_t, 4> xs = {
            aabb.get_vertex(0), aabb.get_vertex(3),
            aabb.get_vertex(5), aabb.get_vertex(6)
        };

        auto result = srph::optimise::nelder_mead(f, xs);
        x = result.x;
        depth = std::abs(result.fx);
        hit = result.fx < 0;
    }

    if (hit){
        // choose best normal based on smallest second derivative
        x_a = a->to_local_space(x);
        x_b = b->to_local_space(x);
        
        auto ja = a->get_sdf()->jacobian(x_a);
        auto jb = b->get_sdf()->jacobian(x_b);

        if (vec::p_norm<1>(ja) <= vec::p_norm<1>(jb)){
            n = a->get_rotation() * a->get_sdf()->normal(x_a);
        } else {
            n = b->get_rotation() * b->get_sdf()->normal(x_b);
        }

        // find contact surface
        auto t1 = vec::tangent(n);
        auto t2 = vec::cross(n, t1);

        aabb2_t box;

        for (int i = 0; i < 8; i++){
            auto v = aabb.get_vertex(i) - x;
            box.capture_point(vec2_t(vec::dot(v, t1), vec::dot(v, t2)));
        }
  
        int n = 4;
    
        vec3_t c;
        int cs = 0;

        for (int i = 0; i <= n; i++){
            for (int j = 0; j <= n; j++){
                vec2_t ij = box.get_min() + box.get_size() * vec2_t(i, j) * 2.0 / n;
                vec3_t v = x + t1 * ij[0] + t2 * ij[1];
                
                vec3_t va = a->to_local_space(v);
                vec3_t vb = b->to_local_space(v);

                if (a->get_sdf()->phi(va) < 0 && b->get_sdf()->phi(vb) < 0){
                    c += v;
                    cs++;
                }
            }
        }

        if (cs != 0){
            x = c / cs;
        }
        
        // find relative velocity at point 
        std::cout << "vb.y = " << b->get_velocity(x)[1] << std::endl;
        vr = a->get_velocity(x) - b->get_velocity(x);
    }
}

void srph::collision_t::resting_correct(){
    std::cout << "resting contact" << std::endl;
    /*
    auto aa = a->get_acceleration(x);
    auto ab = b->get_acceleration(x);
    double ca = vec::dot(aa, n) - vec::dot(ab, n);    
   
 
    if (ca > 0){
        std::cout << "resting correct" << std::endl;
        double sm = a->get_mass() + b->get_mass(); 
        auto d = ca * n / sm;
        a->constrain_acceleration(-d * b->get_mass()); 
        b->constrain_acceleration( d * a->get_mass()); 
    } 
    */
}

void srph::collision_t::colliding_correct(){
    // extricate matters 
    double sm = a->get_mass() + b->get_mass();
    a->translate(-depth * n * b->get_mass() / sm);
    b->translate( depth * n * a->get_mass() / sm);

    // calculate collision impulse magnitude
    auto mata = a->get_material(x_a);
    auto matb = b->get_material(x_b);

    double CoR = std::max(mata.restitution, matb.restitution);

    double jr = (1.0 + CoR) * vec::dot(vr, n) / (
        1.0 / a->get_mass() + a->get_inverse_angular_mass(x, n) +
        1.0 / b->get_mass() + b->get_inverse_angular_mass(x, n)
    );

    a->apply_impulse_at(-jr * n, x);
    b->apply_impulse_at( jr * n, x);

    // apply friction force
    vec3_t t = vr - vec::dot(vr, n) * n;
    if (t == vec3_t()){
        // no surface friction because impact vector is perpendicular to surface
        return;
    }
    t = vec::normalise(t);
    
    double vrt = vec::dot(vr, t); 
    auto mvta = a->get_mass() * vrt;
    auto mvtb = b->get_mass() * vrt;

    double js = std::max(mata.static_friction,  matb.static_friction ) * jr;
    double jd = std::max(mata.dynamic_friction, matb.dynamic_friction) * jr;

    double ka = -(mvta <= js) ? mvta : jd;
    double kb =  (mvtb <= js) ? mvtb : jd;

    a->apply_impulse_at(ka * t, x);
    b->apply_impulse_at(kb * t, x);
}

void srph::collision_t::correct(){
    auto vrn = vec::dot(vr, n);

    if (vrn > constant::epsilon){
        colliding_correct();
    } else if (vrn > -constant::epsilon){
        resting_correct();
    }
}
