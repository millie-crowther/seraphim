#ifndef SERAPHIM_SDF_H
#define SERAPHIM_SDF_H

#include <functional>
#include <memory>

#include "core/constant.h"
#include "maths/bound.h"

namespace srph {
    template<uint8_t D>
    class sdf_t {
    private:
        struct normal_tree_t {
            bool is_explored;
            normal_tree_t * parent;
            bound_t<double, D> normals;
            std::vector<normal_tree_t> children;

            normal_tree_t(normal_tree_t * _parent) : is_explored(false), parent(_parent) {}

            void propagate(){
                auto f = [](const normal_tree_t & x){ return x.is_explored; };
                is_explored = std::all_of(children.begin(), children.end(), f);

                for (const normal_tree_t & child : children){
                    normals |= child.normals;
                }

                // check for planar cell
                if (is_explored && vec::length(normals.get_width()) < constant::epsilon){
                    children.clear();
                }

                if (parent != nullptr){
                    parent->propagate();
                }
            }

            void explore(
                sdf_t<D> & sdf, const bound_t<double, D> & node_bounds, 
                const bound_t<double, D> & query_bounds
            ){
                vec_t<double, D> m = node_bounds.get_midpoint();
                double r = vec::length(node_bounds.get_width());

                if (r <= constant::epsilon || r < std::abs(sdf.phi(m))){
                    vec_t<double, D> n = sdf.normal(m);
                    normals = bound_t<double, D>(n, n);
                    is_explored = true;

                } else { 
                    children.resize(1 << D, normal_tree_t(this));
                   
                    bool is_updated = false;
                    for (int i = 0; i < (1 << D); i++){
                        if (!children[i].is_explored){
                            bound_t<double, D> subbounds = node_bounds.index_subdivision(i);
                                        
                            if (subbounds.intersects(query_bounds)){
                                is_updated = true;
                                children[i].explore(sdf, subbounds, query_bounds);
                            }
                        }
                    }

                    if (is_updated){
                        propagate();
                    }
                }
            }

            bound_t<double, D> get_normal_range(
                sdf_t<D> & sdf, const bound_t<double, D> & node_bounds, 
                const bound_t<double, D> & query_bounds
            ){
                // subdivide if necessary
                if (!is_explored){
                    children.resize(1 << D, normal_tree_t(this));
                }

                // find the child node that contains the bounds you're interested in
                if (!children.empty()){
                    vec_t<double, D> x = query_bounds.get_midpoint();           
                    int i = node_bounds.subdivision_index(x); 
                    bound_t<double, D> subbounds = node_bounds.subdivision(i);
                    if (subbounds.contains(query_bounds)){
                        return children[i].get_normal_range(sdf, subbounds, query_bounds);
                    }
                }

                // check if this node is explored
                if (!is_explored){
                    explore(sdf, node_bounds, query_bounds);    
                }               

                return normals;
            }
        };

        std::unique_ptr<normal_tree_t> root;

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
            return vec::normalise(n);
        }

        virtual bound_t<double, D> get_normal_range(const bound_t<double, D> & bounds){
            if (!root){
                root = std::make_unique<normal_tree_t>(nullptr);
            }

            return bound_t<double, D>(vec_t<double, D>(), vec_t<double, D>());
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

        virtual bool contains(const vec_t<double, D> & x){
            return phi(x) <= 0.0;
        }

        virtual bound_t<double, D> get_bound(){
            return bound_t<double, D>(-constant::rho, constant::rho);
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
