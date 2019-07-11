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
* improve revelator design
    * prone to null references atm
    * can improve
    * fix seg fault on exit
        * definitely something to do with revelator<T>::follower_t destructor
        * probably to do with when resultant shared_ptr is destroyed 
        * destroyed too late? idk
* maybe put the pipelines in their own class

## rendering

### general
* make rays that originate outside octree intersect properly
* camera pitch rotation with mouse
* beam optimisation in a compute shader
    * put a semaphore between compute shader and fragment shader
* figure out the fucking synchronisation on this thing
* implement octree sibling trick

### geometry
* delete bricks which havent been used in a while
* find a way to prune octree interior nodes
    * e.g. a node has eight children, all of whom are far enough away
    * merge children and create one parent 
    * can put in same compute shader as beam optimisation 
* remove parameter of normal map and use flag instead 
* switch to a 64-bit node type
    * means interior nodes can have bricks as well
    * makes culling a lot easier

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
