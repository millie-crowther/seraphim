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


//    if (bound.is_valid()){
//        vec4_t lower(bound.get_lower(), 0.0);
  //      vec4_t upper(bound.get_upper(), constant::sigma);
    //    minimise(bound4_t(lower, upper));
   // }

    std::array<vec3_t, 4> xs = {
        bound.vertex(0), bound.vertex(3),
        bound.vertex(5), bound.vertex(6)
    };

    if (bound.is_valid()){
        auto result = srph::optimise::nelder_mead(f, xs);
        depth = std::abs(result.fx);
        intersecting = result.fx < 0;
        x = result.x;  

//        double t = time_to_collision(bound, constant::sigma);
        auto ttc = [this](const vec3_t & x){ return time_to_collision(x); };
        auto t_result = srph::optimise::nelder_mead(ttc, xs, constant::iota);
        t = std::min(t_result.fx, constant::sigma);
     ///   vec3_t sum_x;
      //  for (auto & min_x : min_xs){
        //    sum_x += min_x;
       // }

       // x = sum_x / min_xs.size();

        x_a = a->to_local_space(x);
        x_b = b->to_local_space(x);
        
        // choose best normal based on smallest second derivative
        auto ja = a->get_sdf()->jacobian(x_a);
        auto jb = b->get_sdf()->jacobian(x_b);

        if (vec::length(ja) <= vec::length(jb)){
            n = a->get_rotation() * a->get_sdf()->normal(x_a);
        } else {
            n = b->get_rotation() * -b->get_sdf()->normal(x_b);
        }
    }
    
    // find relative velocity at point 
    vr = a->get_velocity(x) - b->get_velocity(x);
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

double collision_t::time_to_collision(const vec3_t & x){
    vec3_t xa = a->to_local_space(x);
    double phi_a = a->get_sdf()->phi(xa);
    
    vec3_t xb = b->to_local_space(x);
    double phi_b = b->get_sdf()->phi(xb);
    double phi = phi_a + phi_b;
   
    if (phi <= 0){
        return 0;
    }
 
    vec3_t n = a->get_rotation() * a->get_sdf()->normal(xa);
    vec3_t va = a->get_velocity(x);
    vec3_t vb = b->get_velocity(x);
    vec3_t vr = va - vb;
    double vrn = vec::dot(vr, n);

    if (vrn <= 0){
        return std::numeric_limits<double>::max();
    }

    // estimate of time to collision
    return phi / vrn;        
}
/*
double collision_t::time_to_collision(const bound3_t & bound, double upper_t){
    // take upper bound from sample at centre
    vec3_t c = bound.get_midpoint();
    double tc = time_to_collision(c);
    upper_t = std::min(upper_t, tc);   

    // check if region is unit size
    double r = vec::length(bound.get_width());
    if (r < constant::epsilon || upper_t < constant::iota){
        return tc;
    }
    
    // only check points on the surface of one of the substances
    vec3_t xa = a->to_local_space(c);
    double phi_a = a->get_sdf()->phi(xa);
    if (std::abs(phi_a) > r){
        return upper_t;
    }
    
    // check lower bound on time to collision in this volume
    // TODO: check upper bound on time to collision? 
    bound3_t vba = a->velocity_bounds(bound, interval_t<double>(0, upper_t));   
    bound3_t vbb = b->velocity_bounds(bound, interval_t<double>(0, upper_t));  
    bound3_t vrb = vba - vbb; 
    double upper_v = vec::dot(vrb, vrb).get_upper();
    
    if (upper_v < constant::epsilon){
        return upper_t;
    }

    vec3_t xb = b->to_local_space(c);
    double phi_b = b->get_sdf()->phi(xb);
    double phi = phi_a + phi_b;
    double lower_t = (phi - r * 2) / std::sqrt(upper_v);
    if (lower_t >= upper_t){
        return upper_t;
    }
    
    // recurse 
    // TODO: choose which half to do first based on heuristic
    auto bs = bound.bisect();
    double t1_est = time_to_collision(bs.first.get_midpoint()); 
    double t2_est = time_to_collision(bs.first.get_midpoint()); 
    upper_t = std::min(upper_t, std::min(t1_est, t2_est));

    bound3_t b1 = t1_est < t2_est ? bs.first  : bs.second;
    bound3_t b2 = t1_est < t2_est ? bs.second : bs.first;
    
    double t1 = time_to_collision(b1, upper_t);

    if (t1 < constant::iota){
        return t1;
    }

    double t2 = time_to_collision(b2, std::min(upper_t, t1));
    return std::min(t1, t2);
}
*/
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
   // bound3_t bound = a->get_bound() & b->get_bound();
    //find_contact_points(bound);

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


bool srph::collision_t::comparator_t::operator()(const collision_t & a, const collision_t & b){
    return a.t < b.t;
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

    std::cout << "collision at: " << bound.get_midpoint() << std::endl;
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

void collision_t::find_contact_points(const bound3_t & bound){
    vec3_t x = bound.get_midpoint();
    double r = vec::length(bound.get_width());

    vec3_t x_a   = a->to_local_space(x);
    double phi_a = a->get_sdf()->phi(x_a); 
    
    if (std::abs(phi_a) > r){
        return;
    }
 
    vec3_t x_b   = b->to_local_space(x);
    double phi_b = b->get_sdf()->phi(x_b); 

    if (std::abs(phi_b) > r){
        return;
    }

    for (const vec3_t & c : contact_points){
        bool is_contained = true;
        for (int i = 0; i < 8; i++){
            vec3_t vertex = bound.vertex(i);
            if (vec::length(vertex - c) > solution_density){
                is_contained = false;
                break;
            }

            if (is_contained){
                return;
            }
        }
    }

    if (r < constant::epsilon){
        // TODO check if incoming
        contact_points.push_back(x);
        return;
    } 

    auto bs = bound.bisect();
    find_contact_points(bs.first);
    find_contact_points(bs.second);
}
