#include "physics/collision.h"

#include "maths/optimise.h"

#include <map>
#include <queue>

using namespace srph;

srph::collision_t::collision_t(matter_t * a, matter_t * b){
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
   
    aabb3_t aabb = a->get_moving_aabb(constant::sigma) && b->get_moving_aabb(constant::sigma);
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

        vec4_t region_min(aabb.get_min()[0], aabb.get_min()[1], aabb.get_min()[2], 0.0);
        vec4_t region_max(aabb.get_max()[0], aabb.get_max()[1], aabb.get_max()[2], constant::sigma);
        
        minimise(aabb4_t(region_min, region_max));
    }
    
    // find relative velocity at point 
    vr = a->get_velocity_after(x, 0) - b->get_velocity_after(x, 0);

    // check to see if anticipated
    if (aabb.is_valid() && !intersecting){
        auto vrn = vec::dot(vr, n);
       
        if (vrn > 0){
            double distance = a->get_sdf()->phi(x_a) + b->get_sdf()->phi(x_b);

            if (distance > 0){
                t = distance / vrn;
                anticipated = t <= constant::sigma;
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
  /*
    auto aa = a->get_acceleration(x);
    auto ab = b->get_acceleration(x);
    double ca = vec::dot(aa, n) - vec::dot(ab, n);    
 
    if (ca > 0){
        double sm = a->get_mass() + b->get_mass(); 
        auto d = ca * n / sm;
        a->constrain_acceleration(-d * b->get_mass()); 
        b->constrain_acceleration( d * a->get_mass());
    }
    */ 
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
    vr = a->get_velocity_after(x, 0) - b->get_velocity_after(x, 0);
   // std::cout << "xb = " << x_b << std::endl;
        
 
    auto vrn = vec::dot(vr, n);

    if (vrn > constant::epsilon){
        colliding_correct();
    } else {
        resting_correct();
    }
}

void srph::collision_t::minimise(const aabb4_t & initial_region){
    std::queue<aabb4_t> queue;
    queue.push(initial_region);

    std::vector<aabb4_t> solutions;
    std::vector<aabb4_t> sing_solns;
    double upper_t = constant::sigma;

    while (!queue.empty()){
        aabb4_t region = queue.front();
        queue.pop();

        if (!satisfies_constraints(region, upper_t, sing_solns)){
            continue;
        } 

        std::cout << "region size: "<< region.get_size() << std::endl;

        if (should_accept_solution(region)){
            solutions.push_back(region);
            
            if (!contains_unique_solution(region)){
                sing_solns.push_back(region);
            }

            upper_t = std::min(upper_t, upper_bound_t(region));

            auto too_late = [upper_t](const aabb4_t & y){
                return y.get_min()[3] > upper_t + constant::iota;
            };
                
            std::remove_if(solutions.begin(),  solutions.end(),  too_late);
            std::remove_if(sing_solns.begin(), sing_solns.end(), too_late);
        } else {
            auto subregions = subdivide(region);
            queue.push(subregions.first);
            queue.push(subregions.second);
        }
    }
}

bool srph::collision_t::comparator_t::operator()(const collision_t & a, const collision_t & b){
    return a.t < b.t;
}

double srph::collision_t::lower_bound_t(const aabb4_t & region) const {
    // TODO
    return 0.0;
}

double srph::collision_t::upper_bound_t(const aabb4_t & region) const {
    // TODO
    return constant::sigma;
}

bool srph::collision_t::satisfies_constraints(
    const aabb4_t & region, double upper_t, const std::vector<aabb4_t> & sing_solns
) const {
    // is solution too late in time
    if (lower_bound_t(region) > upper_t + constant::iota){
        return false;
    }

    // is solution too densely packed
//    auto is_too_dense = [region, this](const aabb4_t & y){
 //       vec4_t size = (region || y).get_size();
 //       return vec::length(vec3_t(size[0], size[1], size[2])) * 2 < solution_density;
//    };

 //   if (std::any_of(sing_solns.begin(), sing_solns.end(), is_too_dense)){
 //       return false;
  //  }
   
    // is too far from surface
    double t1 = region.get_min()[3];
    double t2 = region.get_max()[3];
    vec4_t c = region.get_centre();
    vec3_t x = vec3_t(c[0], c[1], c[2]);
    vec4_t size1 = region.get_size();
    vec3_t size = vec3_t(size1[0], size1[1], size1[2]);
    
    transform_t tfa = a->get_transform_after(t1);
    vec3_t x_a = tfa.to_local_space(x);
    vec3_t v_a = a->get_velocity_after(x, t1);
    double distance_a = vec::length(v_a) * (t2 - t1);

    if (std::abs(a->get_sdf()->phi(x_a) - distance_a > vec::length(size))){
        return false;
    } 

    transform_t tfb = b->get_transform_after(t1);
    vec3_t x_b = tfb.to_local_space(x);
    vec3_t v_b = b->get_velocity_after(x, t1);
    double distance_b = vec::length(v_b) * (t2 - t1);

    if (std::abs(b->get_sdf()->phi(x_b) - distance_b > vec::length(size))){
        return false;
    } 


    // normals not anti-parallel    
    // TODO

    // incoming collision constraint
    // TODO
 
    return true;
}

bool srph::collision_t::should_accept_solution(const aabb4_t & region) const {
    // TODO: accept at higher resolutions
    vec4_t size = region.get_size();
    if ( 
        vec::length(vec3_t(size[0], size[1], size[2])) > constant::epsilon ||
        size[3] * 2 > constant::iota
    ){
        return false;
    }

    vec4_t c4 = region.get_centre();
    vec3_t centre(c4[0], c4[1], c4[2]);
   
    auto tfa = a->get_transform_after(c4[3]);
    auto tfb = b->get_transform_after(c4[3]);
 
    vec3_t c_a = tfa.to_local_space(centre);
    vec3_t c_b = tfb.to_local_space(centre);

    double phi_a = std::abs(a->get_sdf()->phi(c_a)); 
    double phi_b = std::abs(b->get_sdf()->phi(c_b));

    return std::max(phi_a, phi_b) < constant::epsilon; 
}

std::pair<aabb4_t, aabb4_t> srph::collision_t::subdivide(const aabb4_t & region) const {
    // TODO cleverer subdivision
    vec4_t size = region.get_size();
    size[3] *= constant::epsilon / constant::iota;

    int max_i = 0;
    for (int i = 1; i < 4; i++){
        if (size[i] > size[max_i]){
            max_i = i;
        }
    }
    
    vec4_t min = region.get_min();
    vec4_t max = region.get_max();

    min[max_i] = region.get_centre()[max_i];
    max[max_i] = region.get_centre()[max_i];

    return std::make_pair(aabb4_t(region.get_min(), max), aabb4_t(min, region.get_max()));
}

bool srph::collision_t::contains_unique_solution(const aabb4_t & region) const {
    // TODO
    return false;
}
