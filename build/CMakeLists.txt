cmake_minimum_required(VERSION 3.7)
set (CMAKE_CXX_STANDARD 17)
project (blaspheme)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package ( Threads REQUIRED )

set(VULKAN_SDK_PATH "$ENV{HOME}/VulkanSDK/1.1.101.0/x86_64")
set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/include")
set(Vulkan_LIBRARY "${VULKAN_SDK_PATH}/lib/libvulkan.so") # for macOS, .dylib
find_package(Vulkan REQUIRED)

add_subdirectory("glfw")

set(INCLUDE_DIR
    ../include
    VulkanMemoryAllocator/src
)

include_directories(${INCLUDE_DIR})

set(SOURCES
    ../src/main.cpp
  
    ../src/core/blaspheme.cpp
    ../src/core/sdf.cpp
    
    ../src/lib.cpp
    ../src/input.cpp
    ../src/buffer.cpp
    
    ../src/render/renderer.cpp
    ../src/render/image.cpp
    ../src/render/camera.cpp
    
    ../src/maths/maths.cpp
    ../src/maths/quat.cpp
    
    ../src/scene/transform.cpp

    ../src/input/keyboard.cpp
)

add_executable(blaspheme ${SOURCES})
target_link_libraries(blaspheme Vulkan::Vulkan)
target_link_libraries(blaspheme glfw)