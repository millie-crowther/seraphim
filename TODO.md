# TODO

## general

* make `image_t` use Vulkan Memory Allocator 
    * figure out why it blanks the screen
    * problem may go away if you remove depth resources (see below)
* check if `vkQueueWaitIdle` in vk_utils post commands is bad
    * remove if necessary
* put a FPS counter in the title
* improve design of `allocator_t` 
    * does the job
    * kind of hacky
    * at least it isnt a singleton lol
    * need to investigate how flexible `VkCommandPool` and `VkQueue` need to be
    * pretty sure `VmaAllocator`, `VkDevice` and `VkPhysicalDevice` aren't gonna change
* maybe tidy internals of renderer class

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
* determine best way to animate octree
* lower granularity of octree at distance from camera
    * increase granularity when you get closer (flag for LOD?)
* delete bricks which havent been used in a while
    * record when a brick is used
* make CPU-side buffers for octree a single `std::vector<uint8_t>`
* remove keyboard class from renderer ( only there for testing purposes)
* fix holes at volumes where sdfs intersect

### texturing
* texture surface
* sort out a system for allocation of patches on texture

### lighting
* physically-based shading approach

## physics
1. find good method for partitioning colliders to reach `O(n * log(n))` complexity
2. implement rigidbody physics
3. material physics
    * rigid
    * Fluid
    * Sand?
    * Snow?

## logic
1. test `scheduler_t` to make sure it even works
2. make a coroutine system of some kind
