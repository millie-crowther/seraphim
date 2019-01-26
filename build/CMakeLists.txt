cmake_minimum_required(VERSION 3.7)
set (CMAKE_CXX_STANDARD 17)
project (blaspheme)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package ( Threads REQUIRED )

#set(VULKAN_SDK_PATH /Users/amelia/vulkansdk-macos-1.1.92.1/macOS)
set(VULKAN_SDK_PATH /home/millie/VulkanSDK/1.1.73.0/x86_64)
set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/include")
set(Vulkan_LIBRARY "${VULKAN_SDK_PATH}/lib/libvulkan.so") # for macOS, .dylib
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
    ../src/lib.cpp
    ../src/buffer.cpp
    ../src/vk_utils.cpp
    ../src/render/renderer.cpp
    ../src/render/image.cpp
    ../src/render/mesh.cpp
    ../src/maths/maths.cpp
    ../src/bounds.cpp
    ../src/maths/mat.cpp
    ../src/render/texture.cpp
    ../src/render/camera.cpp
    ../src/scene/transform.cpp
    ../src/render/vertex.cpp
    ../src/input.cpp
    ../src/chalet.cpp
    ../src/maths/quat.cpp
    ../src/render/renderable.cpp
)

add_executable(blaspheme ${SOURCES})
target_link_libraries(blaspheme Vulkan::Vulkan)
target_link_libraries(blaspheme glfw)
