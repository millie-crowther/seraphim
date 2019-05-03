#ifndef N3_TREE_H
#define N3_TREE_H

#include <array>
#include <memory>

#include "renderable.h"

template<unsigned int N>
class n3_tree_h {
private:
    std::array<n3_tree_t<N>, N * N * N> children;
    std::vector<std::weak_ptr<renderable_t>> renderables;

    bool is_homogenous(){
        return false;
    }   

    bool is_empty() {
        return false;
    }
};

typedef n3_tree_h<2> octree_t;

#endif