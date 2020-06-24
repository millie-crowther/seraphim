# seraphim

Seraphim is a Catholic game engine inspired by TempleOS

Features:
* Raycast rendering engine - currently being implemented but should allow for advanced reflective and refractive techniques
* Geometry is lazily streamed into the GPU, allowing it to compacted to fit into shared memory, hopefully allowing real time raycasting
* Precise and efficient physics engine using signed distance fields for collision detection - this is next up to be implemented
* Robust metaphysics engine allowing for more complex reasoning about simulated environment - I have to read a lot more theory before I can correctly implement this, but the outline is currently in the works

## state of the engine

24/06/2020 - Currently hitting a cool eighteen frames per second at 720p on a my laptop's GeForce GTX 860M graphics card. Current optimisation strategy is to restructure geometry lookups to remove a `for` loop in the shader that I think is trashing performance.


## install
`./install.sh`

## usage
`./run.sh`

## dependencies
* vulkan
* glfw
