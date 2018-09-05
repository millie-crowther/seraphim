cmake_minimum_required(VERSION 3.7)
set (CMAKE_CXX_STANDARD 17)
project (blaspheme)

set(VULKAN_SDK_PATH "/home/millie/VulkanSDK/1.1.73.0/x86_64")
set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/include")
set(Vulkan_LIBRARY "${VULKAN_SDK_PATH}/lib/libvulkan.so")
find_package(Vulkan REQUIRED)

add_subdirectory("glfw")

set(INCLUDE_DIR
    ../include
    VulkanMemoryAllocator/src
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
)

add_executable(blaspheme ${SOURCES})
target_link_libraries(blaspheme Vulkan::Vulkan)
target_link_libraries(blaspheme glfw)
