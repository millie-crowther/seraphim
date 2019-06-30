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
* FPS counter is weird. fix it.

## rendering

### general
* make rays that originate outside octree intersect properly

### camera
* add camera_up field to push constants as well
* add camera transform fields directly from a transform of the camera class

### geometry
* implement octree streaming from cpu
    * create gpu-cpu buffer
    * add a CPU phase after rendering to handle requests
* lower granularity of octree at distance from camera
    * increase granularity when you get closer (flag for LOD?)
* delete bricks which havent been used in a while
    * record when a brick is used
* make CPU-side buffers for octree a single `std::vector<uint8_t>`
* fix holes at volumes where sdfs intersect
* fix general holes that appear all over the place
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
* make plane intersect check use normal map 
    * instead of brick normal
    * can get rid of brick normal field, halving its size

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
