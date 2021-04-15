cmake_minimum_required(VERSION 3.7)
set (CMAKE_CXX_STANDARD 17)
project (seraphim)

SET(COMPILER_FLAGS "-fsanitize=address -Wall -Werror -Wfatal-errors")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package ( Threads REQUIRED )

set(VULKAN_SDK_PATH "$ENV{HOME}/VulkanSDK/1.2.131.2/x86_64")
set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/include")
set(Vulkan_LIBRARY "${VULKAN_SDK_PATH}/lib/libvulkan.so") # for macOS, .dylib
find_package(Vulkan REQUIRED)

add_subdirectory("glfw")

set(INCLUDE_DIR
    ../include
)

include_directories(${INCLUDE_DIR})

set(SOURCES
    ../src/main.cpp
  
    ../src/core/command.cpp
    ../src/core/device.cpp
    ../src/core/random.cpp
    ../src/core/scheduler.cpp
    ../src/core/seraphim.cpp
    ../src/core/array.cpp
    ../src/core/set.cpp

    ../src/physics/collision.cpp
    ../src/physics/constraint.cpp    
    ../src/physics/physics.cpp
    ../src/physics/sphere.cpp
    ../src/physics/transform.cpp

    ../src/render/call_and_response.cpp
    ../src/render/camera.cpp
    ../src/render/light.cpp
    ../src/render/renderer.cpp
    ../src/render/swapchain.cpp
    ../src/render/texture.cpp

    ../src/metaphysics/substance.cpp
    ../src/metaphysics/form.cpp
    ../src/metaphysics/matter.cpp

        ../src/maths/bound.cpp
        ../src/maths/optimise.cpp
    ../src/maths/maths.cpp
        ../src/maths/sdf/sdf.cpp
        ../src/physics/broad_phase.cpp
    ../src/maths/sdf/primitive.cpp
    ../src/maths/sdf/platonic.cpp

    ../src/ui/keyboard.cpp
    ../src/ui/resources.cpp
    ../src/ui/window.cpp
    ../src/ui/mouse.cpp
)

add_executable(seraphim ${SOURCES})
target_link_libraries(seraphim Vulkan::Vulkan)
target_link_libraries(seraphim glfw)
