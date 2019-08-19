# TODO

## general

* make `texture_t` use Vulkan Memory Allocator 
    * figure out why it blanks the screen
    * remove some functionality from this class, its intended purpose has shifted slightly,
      leaving some code unused.
* FPS counter is weird.
    * choice of clock?
* improve revelator design
    * prone to null dereferences atm
    * can improve
    * fix seg fault on exit
        * definitely something to do with revelator<T>::follower_t destructor
        * probably to do with when resultant shared_ptr is destroyed 
        * destroyed too late? idk
* maybe put the pipelines in their own class
* camera pitch rotation with mouse
* vector class interior removal of branching using templates
* destruction of descriptor pools needs to be re added!!!!

## rendering

### general
* really just improve design of renderer class
* fix scaling of render texture to screen
* figure out why render texture size is stuck
* apply `restrict` qualifiers to relevant buffers once i can figure out what it does
* make rays that originate outside octree intersect properly
* bake buffer updates into compute command buffers each frame, instead of making a separate command buffer
    * may be able to make SSBOs VMA_USAGE_GPU_ONLY after i do this which is nice
* create octree on cpu side that stores renderables
    * makes lookup of which sdfs to query quicker
    * may also help with collision
* compute shader
    * reduce branching 
    * beam optimisation
    * octree pruning
        * make more sophisticated node merge algorithm (at the moment just copies first child)
    * possibly also animation
        * parallelised construction of second octree based on transforms
        * BVH embedded in static octree?
* remove parameter of normal map and use flag instead 
* switch surface representation to quadratic patch
* make loading in the nodes less visually noticeable
    * effectively, back up one stage in octree lookup if pending data flag seen
* do more rearrangement of octree data
    * lookup by `octree_node_t` not `octree_data_t`
    * remove unnecessary flags
    * put flags that can go in `octree_node_t::flags` there instead of children
* lerp colour and normal values across octree node on hit

### materials
* add textures for other material properties
    * albedo
    * temperature
    * etc

### lighting
* physically-based shading approach
* maybe some sort of generic lighting primitive
    * based on surface area, emissive properties of material etc.
* bounce lighting??
* soft lighting
* ambient occlusion

## physics
1. find good method for partitioning colliders to reach `O(n * log(n))` complexity
2. implement rigidbody physics
3. material physics
    * rigid
    * Fluid
    * Sand?
    * Snow?

## logic
* make a coroutine system of some kind

## UI
* add mouse input
* wrap up all the input classes in one struct
