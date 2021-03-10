#include "core/set.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// this file is adapted from the below referenced java code

/******************************************************************************
 *  Copyright 2002-2020, Robert Sedgewick and Kevin Wayne.
 *
 *  This file is part of algs4.jar, which accompanies the textbook
 *
 *      Algorithms, 4th edition by Robert Sedgewick and Kevin Wayne,
 *      Addison-Wesley Professional, 2011, ISBN 0-321-57351-X.
 *      http://algs4.cs.princeton.edu
 *
 *
 *  algs4.jar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  algs4.jar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with algs4.jar.  If not, see http://www.gnu.org/licenses.
 ******************************************************************************/

#define LEFT 0
#define RIGHT 1

#define RED true
#define BLACK false

static srph_set_node * rotate(srph_set_node * n, int dir);
static void flip_colours(srph_set_node * n);
static srph_set_node * move_red(srph_set_node * n, int dir);

static bool colour(srph_set_node * n){
    return n == NULL ? BLACK : n->colour;
}

void srph_set_create(srph_set * s, srph_set_comparator cmp, uint32_t element_size){
    assert(s != NULL);
    assert(cmp != NULL);
    assert(element_size > 0);

    s->element_size = element_size;
    s->root = NULL;
    s->cmp = cmp;
}

static void node_destroy(srph_set_node * n){
    if (n != NULL){
        for (int i = 0; i < 2; i++){
            node_destroy(n->children[i]);
            n->children[i] = NULL;
        }
        free(n);
    }
}

void srph_set_destroy(srph_set * s){
    node_destroy(s->root);
    s->root = NULL;   
}

void * srph_set_find(srph_set * s, void * key){
    assert(s != NULL);    

    if (key == NULL){
        return NULL;
    }

    srph_set_node * n = s->root;
    int c;
    while (n != NULL){
        c = s->cmp(key, n->data);
        if (c < 0){
            n = n->children[LEFT];
        } else if (c > 0){
            n = n->children[RIGHT];
        } else {
            return n->data;
        }
    }
    
    return NULL;
}

bool srph_set_contains(srph_set * s, void * key){
    return key != NULL && srph_set_find(s, key) != NULL;
}

static srph_set_node * rebalance(srph_set_node * n){
    assert(n != NULL);
    
    if (colour(n->children[RIGHT]) == RED && colour(n->children[LEFT]) == BLACK){
        n = rotate(n, LEFT);
    }

    if (colour(n->children[LEFT]) == RED && colour(n->children[LEFT]->children[LEFT]) == RED){
        n = rotate(n, RIGHT);
    }

    if (colour(n->children[LEFT]) == RED && colour(n->children[RIGHT]) == RED){
        flip_colours(n);
    }

    return n;
}

srph_set_node * insert_helper(srph_set * s, srph_set_node * n, void * key){
    if (n == NULL){
        n = (srph_set_node *) malloc(sizeof(srph_set_node) + s->element_size);
        n->children[LEFT] = NULL;
        n->children[RIGHT] = NULL;
        n->colour = RED;
        memcpy(n->data, key, s->element_size);
        return n;
    }
   
    int c = s->cmp(key, n->data);
    if (c < 0){
        n->children[LEFT]  = insert_helper(s, n->children[LEFT],  key);
    } else if (c > 0){
        n->children[RIGHT] = insert_helper(s, n->children[RIGHT], key);
    } else {
        memcpy(n->data, key, s->element_size);
    }

    return rebalance(n);
}

void srph_set_insert(srph_set * s, void * key){
    assert(s != NULL);

    if (key == NULL){
        return;
    }

    s->root = insert_helper(s, s->root, key);
    s->root->colour = BLACK;
} 

static srph_set_node * delete_min(srph_set_node * n){
    if (n->children[LEFT] == NULL){
        free(n);
        return NULL;
    }

    if (colour(n->children[LEFT]) == BLACK && colour(n->children[LEFT]->children[LEFT]) == BLACK){
        n = move_red(n, LEFT);
    }

    n->children[LEFT] = delete_min(n->children[LEFT]);
    return rebalance(n);
}

static srph_set_node * min_helper(srph_set_node * n){
    assert(n != NULL);

    if (n->children[LEFT] == NULL){
        return n;
    } 

    return min_helper(n->children[LEFT]);
}

static srph_set_node * delete_helper(srph_set * s, srph_set_node * n, void * key){
    if (s->cmp(key, n->data) < 0){
        if (
            colour(n->children[LEFT]) == BLACK &&
            colour(n->children[LEFT]->children[LEFT]) == BLACK
        ){
            n = move_red(n, LEFT);
        }
        n->children[LEFT] = delete_helper(s, n->children[LEFT], key);
    } else {
        if (colour(n->children[RIGHT]) == RED){
            n = rotate(n, RIGHT);
        } 

        if (s->cmp(key, n->data) == 0 && n->children[RIGHT] == NULL){
            free(n);
            return NULL;
        }

        if (
            colour(n->children[RIGHT]) == BLACK && 
            colour(n->children[RIGHT]->children[LEFT]) == BLACK
        ){
            n = move_red(n, RIGHT);
        }

        if (s->cmp(key, n->data) == 0){
            srph_set_node * m = min_helper(n->children[RIGHT]);
            memcpy(n->data, m->data, s->element_size);
            n->children[RIGHT] = delete_min(n->children[RIGHT]);
        } else {
            n->children[RIGHT] = delete_helper(s, n->children[RIGHT], key);
        }
    }

    return rebalance(n);
}

void srph_set_delete(srph_set * s, void * key){
    assert(s != NULL);

    if (!srph_set_contains(s,  key)){
        return;
    }

    if (colour(s->root->children[LEFT]) == BLACK && colour(s->root->children[RIGHT]) == BLACK){
        s->root->colour = RED;
    }

    s->root = delete_helper(s, s->root, key);

    if (s->root != NULL){
        s->root->colour = BLACK;
    }
}

static srph_set_node * rotate(srph_set_node * n, int dir){
    assert(n != NULL && colour(n->children[1 - dir]) == RED);
    
    srph_set_node * x = n->children[1 - dir];
    n->children[1 - dir] = x->children[dir];
    x->children[dir] = n;
    x->colour = colour(x->children[dir]);
    x->children[dir]->colour = RED;
    return x;
}

static void flip_colours(srph_set_node * n){
    assert(n != NULL && n->children[LEFT] != NULL && n->children[RIGHT] != NULL);
    assert(colour(n->children[LEFT]) == colour(n->children[RIGHT]));
    assert(colour(n) != colour(n->children[LEFT]));

    n->colour = !n->colour;
    n->children[LEFT]->colour = !n->children[LEFT]->colour;
    n->children[RIGHT]->colour = !n->children[RIGHT]->colour;
}

static srph_set_node * move_red(srph_set_node * n, int dir){
    assert(n != NULL);
    assert(colour(n) == RED);
    assert(colour(n->children[dir]) == BLACK);
    assert(colour(n->children[dir]->children[LEFT]) == BLACK);

    flip_colours(n);
    
    if (colour(n->children[1 - dir]->children[LEFT]) == RED){
        if (dir == LEFT){ 
            n->children[RIGHT] = rotate(n->children[RIGHT], RIGHT);
        }
        n = rotate(n, dir);
        flip_colours(n);
    }

    return n;
}
