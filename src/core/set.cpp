#include "core/set.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void rotate_left(srph_set_node ** root, srph_set_node * x){
    if (x == NULL || x->right == NULL){
        return ;
    }

    //y stored pointer of right child of x
    srph_set_node * y = x->right;

    //store y's left subtree's pointer as x's right child
    x->right = y->left;

    //update parent pointer of x's right
    if (x->right != NULL){
        x->right->parent = x;
    }

    //update y's parent pointer
    y->parent = x->parent;

    // if x's parent is null make y as root of tree
    if (x->parent == NULL){
        *root = y;
    } else if (x == x->parent->left){
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    // make x as left child of y
    y->left = x;

    //update parent pointer of x
    x->parent = y;
}


static void rotate_right(srph_set_node ** root, srph_set_node * y){
    if (y == NULL || y->left == NULL){
        return;
    }

    srph_set_node * x = y->left;
    y->left = x->right;

    if (x->right != NULL){
        x->right->parent = y;
    }

    x->parent =y->parent;

    if (x->parent == NULL){
        *root = x;
    } else if (y == y->parent->left){
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }

    x->right = y;
    y->parent = x;
}

// Utility function to fixup the Red-Black tree after standard BST insertion
static void rebalance(srph_set_node ** root, srph_set_node * z){
    // iterate until z is not the root and z's parent color is red
    while (z != *root && z != (*root)->left && z != (*root)->right && z->parent->colour == 'R'){
        srph_set_node * y;

        // Find uncle and store uncle in y
        if (z->parent && z->parent->parent && z->parent == z->parent->parent->left){
            y = z->parent->parent->right;
        } else {
            y = z->parent->parent->left;
        }

        // If uncle is RED, do following
        // (i)  Change color of parent and uncle as BLACK
        // (ii) Change color of grandparent as RED
        // (iii) Move z to grandparent
        if (y == NULL){
            z = z->parent->parent;
        } else if (y->colour == 'R'){
            y->colour = 'B';
            z->parent->colour = 'B';
            z->parent->parent->colour = 'R';
            z = z->parent->parent;

        // Uncle is BLACK, there are four cases (LL, LR, RL and RR)
        } else  {
            // Left-Left (LL) case, do following
            // (i)  Swap color of parent and grandparent
            // (ii) Right Rotate Grandparent
            if (z->parent == z->parent->parent->left && z == z->parent->left){
                z->parent->colour = z->parent->parent->colour;
                z->parent->parent->colour = z->parent->colour;
                rotate_right(root, z->parent->parent);
            }

            // Left-Right (LR) case, do following
            // (i)  Swap color of current node  and grandparent
            // (ii) Left Rotate Parent
            // (iii) Right Rotate Grand Parent
            if (
                z->parent && z->parent->parent && 
                z->parent == z->parent->parent->left && z == z->parent->right
            ){
                z->colour = z->parent->parent->colour;
                z->parent->parent->colour = z->colour;
                rotate_left(root, z->parent);
                rotate_right(root, z->parent->parent);
            }

            // Right-Right (RR) case, do following
            // (i)  Swap color of parent and grandparent
            // (ii) Left Rotate Grandparent
            if (
                z->parent && z->parent->parent &&
                z->parent == z->parent->parent->right && z == z->parent->right
            ){
                z->parent->colour = z->parent->parent->colour;
                z->parent->parent->colour = z->parent->colour;
                rotate_left(root, z->parent->parent);
            }

            // Right-Left (RL) case, do following
            // (i)  Swap color of current node  and grandparent
            // (ii) Right Rotate Parent
            // (iii) Left Rotate Grand Parent
            if (
                z->parent && z->parent->parent && z->parent == z->parent->parent->right &&
                z == z->parent->left
            ){
                z->colour = z->parent->parent->colour;
                z->parent->parent->colour = z->colour;
                rotate_right(root, z->parent);
                rotate_left(root, z->parent->parent);
            }
        }
    }
    (*root)->colour = 'B'; //keep root always black
}

static srph_set_node * set_find_helper(srph_set * set, srph_set_node * node, void * key){
    int c = set->cmp(node->data, key);
    if (c < 0 && node->left != NULL){
        return set_find_helper(set, node->left, key);
    } else if (c > 0 && node->right != NULL){
        return set_find_helper(set, node->right, key);
    }
    
    return node;
}

void * srph_set_find(srph_set * s, void * key){
    srph_set_node * n = set_find_helper(s, s->root, key);
    return s->cmp(n->data, key) == 0 ? n->data : NULL;
}

bool srph_set_contains(srph_set * s, void * key){
    return srph_set_find(s, key) != NULL;
}


// Utility function to insert newly node in RedBlack tree
void srph_set_insert(srph_set * set, void * key){
    srph_set_node * parent = set_find_helper(set, set->root, key);
    if (set->cmp(parent->data, key) == 0){
        memcpy(parent->data, key, set->element_size);
        return;
    }

    // Allocate memory for new node
    srph_set_node * new_node = (srph_set_node *) malloc(sizeof(srph_set_node) + set->element_size);
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->parent = NULL;
    memcpy(new_node->data, key, set->element_size);

    //if root is null make z as root
    if (set->root == NULL){
        new_node->colour = 'B';
        set->root = new_node;
    } else {
        new_node->parent = parent;
        if (set->cmp(parent->data, key) < 0){
            parent->left = new_node;
        } else {
            parent->right = new_node;
        }
        new_node->colour = 'R';

        rebalance(&set->root, new_node);
    }
}

void srph_set_create(srph_set * s, uint32_t element_size){
    s->element_size = element_size;
    s->root = NULL;
}

static void node_destroy(srph_set_node * n){
    if (n != NULL){
        node_destroy(n->left);
        n->left = NULL;
        node_destroy(n->right);
        n->right = NULL;
        free(n);
    }
}

void srph_set_destroy(srph_set * s){
    node_destroy(s->root);
    s->root = NULL;   
} 
