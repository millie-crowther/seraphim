# TODO

## general

* make `image_t` use Vulkan Memory Allocator 
    * figure out why it blanks the screen
* check if `vkQueueWaitIdle` in vk_utils post commands is bad
    * remove if necessary
* improve design of `allocator_t` 
    * does the job
    * kind of hacky
    * at least it isnt a singleton lol
    * need to investigate how flexible `VkCommandPool` and `VkQueue` need to be
    * pretty sure `VmaAllocator`, `VkDevice` and `VkPhysicalDevice` aren't gonna change
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
* investigate putting the whole thing in a single compute shader
    * possible performance impact (graphics pipeline exists for a reason i guess)
    * makes design much simpler
    * complexity of having both compute and frag shader eliminated
    * cleverer things possible with more control
    * e.g. one 64x64 work group for a patch of screen, 2D array of work groups to cover scene
        * iterative beam optimisation
* destruction of descriptor pools needs to be re added!!!!

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
* figure out the fucking synchronisation on this thing
    * sync between compute and fragment shaders
    * sync with buffer writes (screen blanking sometimes)
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
