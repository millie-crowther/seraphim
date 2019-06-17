# TODO

## general

* make `image_t` use Vulkan Memory Allocator 
    * figure out why it blanks the screen
    * problem may go away if you remove depth resources (see below)
* remove `blaspheme_t::get_device()` and `blaspheme_t::get_physical_device()`
    * they're hacky
    * singleton pattern is gross
    * not necessary anyway
    * they're only still in buffer.cpp and image.cpp afaict
* check if `vkQueueWaitIdle` in vk_utils post commands is bad
    * remove if necessary
* put a FPS counter in the title
* make sdf_t class dimension-generic

## rendering

### general
* make rays that originate outside octree intersect properly
* make the collection of renderables a set not a vector (order unimportant)

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
* fix double free on renderable transform
* make renderables and SDFs use transform class for positioning rather than internally
* make CPU-side buffers for octree a single `std::vector<uint8_t>`
* reduce plane size from `f32vec4_t` to `f32vec3_t` (one element of normals redundant)
    * perhaps reduce further e.g. `float` -> `int16_t`
    * only problem is most things need to be 4 byte aligned 
* lerp normals
* remove keyboard class from renderer ( only there for testing purposes)
* fix holes at volumes where renderables intersect
* fix holes in surface (due to inadequate surface representation)
    * cleverer plane estimation?
    * polynomial fit? (can be guaranteed to have no holes iirc)
* remove `lambda_sdf_t` from all mutator / compositional SDFs

### colouring
1. determine best way to uv map surface
2. texture surface
3. reflective and refractive materials

### lighting
* progress to physically-based shading approach
* global illumination approximants
    * AO
    * bounce lighting
    * more sophisticated (more general?) light primitive

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
