#ifndef SERAPHIM_SDF_H
#define SERAPHIM_SDF_H

#include <functional>
#include <memory>

#include "core/constant.h"

#include "maths/bound.h"

namespace srph {
    template<uint8_t D>
    class sdf_t {
    protected:
        // protected constructor for abstract class
        sdf_t(){}

    public:
        // virtual accessors 
        virtual double phi(const vec_t<double, D> & x) = 0;

        virtual vec_t<double, D> normal(const vec_t<double, D> & x){
            vec_t<double, D> n;
            for (int i = 0; i < D; i++){
                vec_t<double, D> axis;
                axis[i] = constant::epsilon;
                n[i] = phi(x + axis) - phi(x - axis);
            }   
            return n / (2.0 * constant::epsilon);
        }

        matrix_t<double, D, D> jacobian(const vec_t<double, D> & x){
            matrix_t<double, D, D> j;

            for (int column = 0; column < D; column++){
                vec_t<double, D> axis;
                axis[column] = constant::epsilon;
                auto n = normal(x + axis) - normal(x - axis);
                n /= 2 * constant::epsilon;

                for (int row = 0; row < D; row++){
                    j.set(row, column, n[row]);
                }
            }

            return j;
        }

        double project(const vec_t<double, D> & d){
            vec_t<double, D> x = vec::normalise(d);
            double scale = constant::rho;

            while (true){
                double p = phi(x * scale);

                // TODO: better criteria for acceptance?
                //       we want theta to be small enough that theta = cos(theta) = 1    
                if (p > constant::rho){
                    return scale - p;
                }
    
                scale *= 1.5;
            }
        }

        vec3_t support(const vec_t<double, D> & d){
            vec_t<double, D> x = vec::normalise(d) * constant::rho;

            while (true){
                double p = phi(x);

                // TODO: better criteria for acceptance?
                //       we want theta to be small enough that theta = cos(theta) = 1    
                if (p > constant::rho){
                    return x - normal(x) * p;
                }
    
                x *= 1.5;
            }
        }

        virtual bool contains(const vec_t<double, D> & x){
            return phi(x) <= 0.0;
        }

        virtual bound_t<double, D> get_bound(){
            bound_t<double, D> bound;

            for (int i = 0; i < D; i++){
                vec_t<double, D> axis;
                axis[i] = 1;
                bound[i] = interval_t<double>(-project(-axis), project(axis));
            }

            return bound;
        }

        virtual double get_volume(){
            return 0.0;
        }

        virtual vec_t<double, D> get_uniform_centre_of_mass(){
            return vec_t<double, D>();
        }

        // TODO: this isn't dimension independent! maybe it should be in a sub class?
        //       only problem is that it fucks with inheritance with sub classes that 
        //       are already dimension independent. Fuck OOP sometimes, honestly.  
        virtual mat3_t get_uniform_inertia_tensor(double mass){
            // TODO
            return mat3_t::identity(); 
        }
    };

    using sdf2_t = sdf_t<2>;
    using sdf3_t = sdf_t<3>;
}

#endif
