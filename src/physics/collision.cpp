#include "physics/collision.h"

#include "maths/matrix.h"
#include "maths/interval.h"

#include "maths/optimise.h"
#include "maths/sdf/platonic.h"

#include <map>
#include <queue>

using namespace srph;

srph::collision_t::collision_t(matter_t * a, matter_t * b){
    this->a = a;
    this->b = b;
    intersecting = false;
    anticipated = false;   
 
    auto f = [a, b](const vec3_t & x) -> double {
        vec3_t x_a   = a->to_local_space(x);
        double phi_a = a->get_sdf()->phi(x_a); 
        
        vec3_t x_b   = b->to_local_space(x);
        double phi_b = b->get_sdf()->phi(x_b); 

        return std::max(phi_a, phi_b);
    };

    bound3_t bound = a->get_moving_bound(constant::sigma) & b->get_moving_bound(constant::sigma);

    auto get_vertex = [bound](int i){
        vec3_t x = bound.get_lower();
        for (int j = 0; j < 3; j++){
            if (i & (1 << j)){
                x[j] = bound[j].get_upper();
            }
        }
        return x;
    };

    if (bound.is_valid()){
        vec4_t lower(bound.get_lower(), 0.0);
        vec4_t upper(bound.get_upper(), constant::sigma);
        minimise(bound4_t(lower, upper));
    }

    std::array<vec3_t, 4> xs = {
        get_vertex(0), get_vertex(3),
        get_vertex(5), get_vertex(6)
    };

    if (!min_xs.empty()){
        auto result = srph::optimise::nelder_mead(f, xs);
        depth = std::abs(result.fx);
        intersecting = result.fx < 0;
        
        vec3_t sum_x;
        for (auto & min_x : min_xs){
            sum_x += min_x;
        }

        x = sum_x / min_xs.size();

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
    vr = a->get_velocity_after(x, 0) - b->get_velocity_after(x, 0);

    // check to see if anticipated
    if (bound.is_valid() && !intersecting){
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

void srph::collision_t::correct(){
    // extricate matters 
    double sm = a->get_mass() + b->get_mass();
    a->translate(-depth * n * b->get_mass() / sm);
    b->translate( depth * n * a->get_mass() / sm);
    
    auto vrn = vec::dot(vr, n);

    if (vrn > constant::epsilon){
        colliding_correct();
    } else {
        resting_correct();
    }
}

void srph::collision_t::minimise(const bound4_t & initial_bound){
    auto cmp = [](const bound4_t & a, const bound4_t & b){
        return a[3].get_upper() < b[3].get_upper();
    };
    std::priority_queue<bound4_t, std::vector<bound4_t>, decltype(cmp)> queue(cmp);
    queue.push(initial_bound);

    std::vector<bound4_t> solutions;
    std::vector<bound4_t> sing_solns;
    double upper_t = constant::sigma;

    while (!queue.empty()){
        bound4_t bound = queue.top();
        queue.pop();

        if (!satisfies_constraints(bound, upper_t, sing_solns)){
            continue;
        } 

        if (should_accept_solution(bound)){
            solutions.push_back(bound);
            
            if (!contains_unique_solution(bound)){
                sing_solns.push_back(bound);
            }

            upper_t = std::min(upper_t, bound[3].get_upper());

            auto too_late = [upper_t](const bound4_t & y){
                return y[3].get_lower() > upper_t + constant::iota;
            };
                
            std::remove_if(solutions.begin(),  solutions.end(),  too_late);
            std::remove_if(sing_solns.begin(), sing_solns.end(), too_late);
        } else {
            auto subregions = subdivide(bound);
            queue.push(subregions.first);
            queue.push(subregions.second);
        }
    }

    for (auto & solution : solutions){
        vec4_t centre = solution.get_midpoint();
        min_xs.emplace_back(centre[0], centre[1], centre[2]);
    }
    min_t = upper_t;
}

bool srph::collision_t::comparator_t::operator()(const collision_t & a, const collision_t & b){
    return a.t < b.t;
}

bool srph::collision_t::satisfies_constraints(
    const bound4_t & bound, double upper_t, const std::vector<bound4_t> & sing_solns
) const {
    // is solution too late in time
    if (bound[3].get_lower() > upper_t + constant::iota){
        return false;
    }

    // is solution too densely packed
    auto is_too_dense = [bound, this](const bound4_t & y){
        vec4_t size = (bound | y).get_width();
        return vec::length(vec3_t(size[0], size[1], size[2])) * 2 < solution_density;
    };

    if (std::any_of(sing_solns.begin(), sing_solns.end(), is_too_dense)){
        return false;
    }
   
    // is too far from surface
    double t1 = bound[3].get_lower();
    double t2 = bound[3].get_upper();
    vec4_t c1 = bound.get_midpoint();
    vec3_t c = vec3_t(c1[0], c1[1], c1[2]);
    vec4_t size1 = bound.get_width();
    vec3_t size = vec3_t(size1[0], size1[1], size1[2]);
    
    transform_t tfa = a->get_transform_after(t1);
    vec3_t c_a = tfa.to_local_space(c);
    vec3_t v_a = a->get_velocity_after(c, t1);
    double distance_a = vec::length(v_a) * (t2 - t1);

    if (std::abs(a->get_sdf()->phi(c_a) - distance_a > vec::length(size))){
        return false;
    } 

    transform_t tfb = b->get_transform_after(t1);
    vec3_t c_b = tfb.to_local_space(c);
    vec3_t v_b = b->get_velocity_after(c, t1);
    double distance_b = vec::length(v_b) * (t2 - t1);

    if (std::abs(b->get_sdf()->phi(c_b) - distance_b > vec::length(size))){
        return false;
    } 

    // normals not anti-parallel    
    
    // incoming collision constraint
    // TODO
 
    return true;
}

bool srph::collision_t::should_accept_solution(const bound4_t & bound) const {
    // TODO: accept at higher resolutions
    vec4_t size = bound.get_width();
    if ( 
        vec::length(vec3_t(size[0], size[1], size[2])) * 2 > constant::epsilon ||
        size[3] * 2 > constant::iota
    ){
        return false;
    }

    std::cout << "collision at: " << bound.get_width() << std::endl;
    return true;
/*
    vec4_t c4 = region.get_centre();
    vec3_t centre(c4[0], c4[1], c4[2]);
   
    auto tfa = a->get_transform_after(c4[3]);
    auto tfb = b->get_transform_after(c4[3]);
 
    vec3_t c_a = tfa.to_local_space(centre);
    vec3_t c_b = tfb.to_local_space(centre);

    double phi_a = std::abs(a->get_sdf()->phi(c_a)); 
    double phi_b = std::abs(b->get_sdf()->phi(c_b));

    return std::max(phi_a, phi_b) < constant::epsilon; 
*/
}

std::pair<bound4_t, bound4_t> srph::collision_t::subdivide(const bound4_t & bound) const {
    // TODO cleverer subdivision
    vec4_t size = bound.get_width();
    size[3] *= constant::epsilon / constant::iota;

    int max_i = 0;
    for (int i = 1; i < 4; i++){
        if (size[i] > size[max_i]){
            max_i = i;
        }
    }
    
    vec4_t min = bound.get_lower();
    vec4_t max = bound.get_upper();

    min[max_i] = bound.get_midpoint()[max_i];
    max[max_i] = bound.get_midpoint()[max_i];

    return std::make_pair(
        bound4_t(bound.get_lower(), max), 
        bound4_t(min, bound.get_upper())
    );
}

bool srph::collision_t::contains_unique_solution(const bound4_t & bound) const {
    // TODO
    return false;
}
