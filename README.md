# seraphim

Seraphim is a Catholic game engine with a few weird and cool fetures: 

* Raycast rendering engine - currently being implemented but should allow for advanced reflective and refractive techniques
* Precise and efficient physics engine using signed distance fields for collision detection - this is next up to be implemented
* Robust metaphysics engine allowing for more complex reasoning about simulated environment - I have to read a lot more theory before I can correctly implement this, but the outline is currently in the works

## state of the engine

22/08/2020 - about 80 FPS at 720p. main problem with rendering is now hash collisions - currently working on basics physics simulation 

## install
`./install.sh`

## usage
`./run.sh`

## dependencies
* vulkan
* glfw
