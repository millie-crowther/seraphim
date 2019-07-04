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
* maybe tidy internals of renderer class
    * make a dedicated swapchain class?
* FPS counter is weird. fix it.
* fix seg fault on exit
    * definitely something to do with revelator<T>::follower_t destructor
    * probably to do with when resultant shared_ptr is destroyed 
    * destroyed too late? idk

## rendering

### general
* make rays that originate outside octree intersect properly

### camera
* camera rotation with mouse

### geometry
* implement octree streaming from cpu
    * add a CPU phase after rendering to handle requests
* increase granularity of coarse octree when you get close
    * may require a LOD flag to be stored in gpu data?
* delete bricks which havent been used in a while
    * put bricks in a queue
    * pop from end when you need a new one
    * simple and cheap LRU approximant
* find a way to prune octree
    * e.g. a node has eight children, all of whom are null
    * merge children and create one null parent 
    * need to keep track of parent relationship??
* Jacobian culling approximation too greedy in some cases
    * maybe monte carlo sampling?
        * 99% of cases will terminate after second iteration so not as expensive as you might think
        * when to stop, though?
        * perhaps statisical analysis of problem will allow a clean solution
        * e.g. when 99% probability of being planar is satisfied
            * even 99% probability will still lead to hundreds of incorrect bricks
            * but not incorrect in a highly visible way i guess
    * fitting of quadratic curve to surface
        * then check if curve is planar
        * computationally, probably just the same as above
        * but without early termination
* reduce onion skinning

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
