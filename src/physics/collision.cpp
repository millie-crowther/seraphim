#include "physics/collision.h"

#include "maths/optimise.h"

#include <map>
#include <queue>

using namespace srph;

srph::collision_t::collision_t(
    double delta_t,
    matter_t * a, matter_t * b
){
    this->a = a;
    this->b = b;
    intersecting = false;
    anticipated = false;   
 
    auto f = [a, b](const vec3_t & x){
        vec3_t x_a   = a->to_local_space(x);
        double phi_a = a->get_sdf()->phi(x_a); 
        
        vec3_t x_b   = b->to_local_space(x);
        double phi_b = b->get_sdf()->phi(x_b); 

        return std::max(phi_a, phi_b);
    };
   
    aabb3_t aabb = a->get_moving_aabb(delta_t) && b->get_moving_aabb(delta_t);
    std::array<vec3_t, 4> xs = {
        aabb.get_vertex(0), aabb.get_vertex(3),
        aabb.get_vertex(5), aabb.get_vertex(6)
    };

    if (aabb.is_valid()){
        auto result = srph::optimise::nelder_mead(f, xs);
        x = result.x;
        depth = std::abs(result.fx);
        intersecting = result.fx < 0;
        
        x_a = a->to_local_space(x);
        x_b = b->to_local_space(x);
        
        // choose best normal based on smallest second derivative
        auto ja = a->get_sdf()->jacobian(x_a);
        auto jb = b->get_sdf()->jacobian(x_b);

        if (vec::p_norm<1>(ja) <= vec::p_norm<1>(jb)){
            n = a->get_rotation() * a->get_sdf()->normal(x_a);
        } else {
            n = b->get_rotation() * -b->get_sdf()->normal(x_b);
        }
    }
    
    // find relative velocity at point 
    vr = a->get_velocity(x) - b->get_velocity(x);

    // check to see if anticipated
    if (aabb.is_valid() && !intersecting){
        auto vrn = vec::dot(vr, n);
       
        if (vrn > 0){
            double distance = a->get_sdf()->phi(x_a) + b->get_sdf()->phi(x_b);

            if (distance > 0){
                t = distance / vrn;
                anticipated = t <= delta_t;
            }
        }
    }
}

bool srph::collision_t::is_intersecting() const {
    return intersecting;
}

bool srph::collision_t::is_anticipated() const {
    return anticipated;
}

double srph::collision_t::get_estimated_time() const {
    return t;
}

vec3_t srph::collision_t::get_position() const {
    return x;
}

std::pair<matter_t *, matter_t *> srph::collision_t::get_matters() const {
    return std::make_pair(a, b);
}

void srph::collision_t::resting_correct(){
    auto aa = a->get_acceleration(x);
    auto ab = b->get_acceleration(x);
    double ca = vec::dot(aa, n) - vec::dot(ab, n);    
 
    if (ca > 0){
        double sm = a->get_mass() + b->get_mass(); 
        auto d = ca * n / sm;
        a->constrain_acceleration(-d * b->get_mass()); 
        b->constrain_acceleration( d * a->get_mass());
    } 
}

void srph::collision_t::minimise(const aabb4_t & initial_region){
    std::queue<aabb4_t> queue;
    queue.push(initial_region);

    std::vector<aabb4_t> solutions;
    std::vector<aabb4_t> singulars;
    double upper_t = constant::sigma;

    while (!queue.empty()){
        aabb4_t region = queue.front();
        queue.pop();
    }
}

void srph::collision_t::colliding_correct(){
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
    if (t != vec3_t()){
        // no surface friction because impact vector is perpendicular to surface
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
}

void srph::collision_t::correct(const vec3_t & adjusted_x){
    // extricate matters 
    double sm = a->get_mass() + b->get_mass();
    a->translate(-depth * n * b->get_mass() / sm);
    b->translate( depth * n * a->get_mass() / sm);
    
//    x = adjusted_x;
    x_a = a->to_local_space(x);
    x_b = b->to_local_space(x);
    vr = a->get_velocity(x) - b->get_velocity(x);
   // std::cout << "xb = " << x_b << std::endl;
        
 
    auto vrn = vec::dot(vr, n);

    if (vrn > constant::epsilon){
        colliding_correct();
    } else {
        resting_correct();
    }
}

bool srph::collision_t::comparator_t::operator()(const collision_t & a, const collision_t & b){
    return a.t < b.t;
}

double srph:collision_t::lower_bound_t(const aabb4_t & region) const {
    return 0.0;
}

double srph::collision_t::upper_bound_t(const aabb4_t & region) const {
    return constant::sigma;
}

bool srph::collision_t::may_contain_collision(const aabb4_t & region) const {
    return true;
}

bool srph::collision_t::should_accept_region(const aabb4_t & region) const {
    return 
        vec::length(vec3_t(region[0], region[1], region[2])) < constant::epsilon &&
        region[3] <= constant::iota;
}

std::pair<aabb4_t, aabb4_t> srph::collision_t::subdivide(const aabb4_t & region) const {
    return std::make_pair(region, region);
} 
