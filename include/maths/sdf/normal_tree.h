#ifndef SERAPHIM_NORMAL_TREE
#define SERAPHIM_NORMAL_TREE

#include "maths/bound.h"
#include "maths/matrix.h"

namespace srph {
    template<int N>
    struct normal_tree_t {
        bool is_explored;
        normal_tree_t<N> * parent;
        bound_t<double, N> normals;
        std::vector<normal_tree_t<N>> children;

        normal_tree_t(normal_tree_t<N> * _parent) : is_explored(false), parent(_parent) {}

        void propagate(){
            auto f = [](const normal_tree_t<N> & x){ return x.is_explored; };
            is_explored = std::all_of(children.begin(), children.end(), f);

            for (const auto & child : children){
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

        template<class SDF>
        void explore(
            SDF & sdf, const bound_t<double, N> & node_bounds, 
            const bound_t<double, N> & query_bounds
        ){
            vec_t<double, N> m = node_bounds.get_midpoint();
            double r = vec::length(node_bounds.get_width());

            if (r <= constant::epsilon || r < std::abs(sdf.phi(m))){
                vec_t<double, N> n = sdf.normal(m);
                normals = bound_t<double, N>(n, n);
                is_explored = true;

            } else { 
                children.resize(1 << N, normal_tree_t<N>(this));
               
                bool is_updated = false;
                for (int i = 0; i < (1 << N); i++){
                    if (!children[i].is_explored){
                        bound_t<double, N> subbounds = node_bounds.index_subdivision(i);
                                    
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

        template<class SDF>
        bound_t<double, N> get_normal_range(
            SDF & sdf, const bound_t<double, N> & node_bounds, 
            const bound_t<double, N> & query_bounds
        ){
            // subdivide if necessary
            if (!is_explored){
                children.resize(1 << N, normal_tree_t<N>(this));
            }

            // find the child node that contains the bounds you're interested in
            if (!children.empty()){
                vec_t<double, N> x = query_bounds.get_midpoint();           
                int i = node_bounds.subdivision_index(x); 
                bound_t<double, N> subbounds = node_bounds.subdivision(i);
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
}

#endif
