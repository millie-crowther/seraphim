# TODO

## general

* make `image_t` use Vulkan Memory Allocator 
    * figure out why it blanks the screen
* FPS counter is weird.
    * choice of clock?
* improve revelator design
    * prone to null references atm
    * can improve
    * fix seg fault on exit
        * definitely something to do with revelator<T>::follower_t destructor
        * probably to do with when resultant shared_ptr is destroyed 
        * destroyed too late? idk
* maybe put the pipelines in their own class
* camera pitch rotation with mouse
* vector class interior removal of branching using templates
* do clever things in compute shader
    * e.g. one 64x64 work group for a patch of screen, 2D array of work groups to cover scene
        * iterative beam optimisation
* destruction of descriptor pools needs to be re added!!!!
* really just improve design of renderer class
* bake buffer updates into compute command buffers each frame, instead of making a separate command buffer
* fix scaling of render texture to screen
* fix occasional crash of engine where it hangs
* figure out why render texture size is stuck
* reduce branching in compute shader
* apply `restrict` qualifiers to relevant buffers once i can figure out what it does

## rendering

### general
* make rays that originate outside octree intersect properly
* compute shader
    * beam optimisation
    * octree pruning
        * prune nodes that haven't been seen in a while
        * prune nodes with too high level of detail 
    * possibly also animation
        * parallelised construction of second octree based on transforms
* implement octree sibling trick
* remove parameter of normal map and use flag instead 
* switch surface representation to quadratic patch

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
