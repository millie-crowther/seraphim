cmake_minimum_required(VERSION 3.7)
set (CMAKE_CXX_STANDARD 17)
project (blaspheme)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package ( Threads REQUIRED )

set(VULKAN_SDK_PATH /Users/amelia/vulkansdk-macos-1.1.92.1/macOS)
set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/include")
set(Vulkan_LIBRARY "${VULKAN_SDK_PATH}/lib/libvulkan.dylib")
find_package(Vulkan REQUIRED)

add_subdirectory("glfw")

set(INCLUDE_DIR
    ../include
    VulkanMemoryAllocator/src
    tinyobjloader
    stb
)

include_directories(${INCLUDE_DIR})

set(SOURCES
    ../src/main.cpp
    ../src/engine.cpp
    ../src/sdf.cpp
    ../src/lib.cpp
    ../src/buffer.cpp
    ../src/vk_utils.cpp
    ../src/renderer.cpp
    ../src/image.cpp
    ../src/mesh.cpp
    ../src/maths.cpp
    ../src/quat.cpp
    ../src/bounds.cpp
    ../src/marching_cubes.cpp
    ../src/mat.cpp
    ../src/texture.cpp
    ../src/camera.cpp
    ../src/transform.cpp
    ../src/vertex.cpp
    ../src/input.cpp
    ../src/prop.cpp
    ../src/effector.cpp
    ../src/actor.cpp
    ../src/chalet.cpp
)

add_executable(blaspheme ${SOURCES})
target_link_libraries(blaspheme Vulkan::Vulkan)
target_link_libraries(blaspheme glfw)
