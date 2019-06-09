# TODO

## general

1. make `image_t` use Vulkan Memory Allocator 
    * figure out why it blanks the screen
    * problem may go away if you remove depth resources (see below)
2. remove `blaspheme_t::get_device()` and `blaspheme_t::get_physical_device()`
    * they're hacky
    * singleton pattern is gross
    * not necessary anyway

## rendering

### general
1. remove depth resources from `renderer_t`
2. allow camera movement

### geometry
* figure out brick approximation
    * single plane? 
    * Polynomial fit?
* implement octree streaming from cpu
    * create gpu-cpu buffer
    * add a CPU phase after rendering to handle requests
* determine best way to animate octree
* lower granularity of octree at distance from camera
* fix double free on renderable transform
* enable rendering of multiple renderables at a time

### colouring
1. determine best way to uv map surface
2. texture surface
3. reflective and refractive materials

### lighting
1. blinn-phong model with only point lights initially
2. progress to physically-based shading approach
3. global illumination approximants
    1. AO
    2. bounce lighting
    3. more sophisticated (more general?) light primitive

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
