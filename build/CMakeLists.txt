cmake_minimum_required(VERSION 3.7)
set (CMAKE_CXX_STANDARD 17)
project (blaspheme)

SET(COMPILER_FLAGS "-pedantic -Wall")
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
    ../build/stb
    VulkanMemoryAllocator/src
)

include_directories(${INCLUDE_DIR})

set(SOURCES
    ../src/main.cpp
    ../src/lib.cpp
  
    ../src/core/blaspheme.cpp
    ../src/core/uuid.cpp
    ../src/core/device.cpp
    
    ../src/render/renderer.cpp
    ../src/render/texture.cpp
    ../src/render/camera.cpp
    ../src/render/swapchain.cpp
    ../src/render/octree.cpp
    ../src/render/substance.cpp
    
    ../src/maths/quat.cpp
    
    ../src/scene/transform.cpp

    ../src/ui/keyboard.cpp
    ../src/ui/resources.cpp
    ../src/ui/window.cpp
    ../src/ui/mouse.cpp

    ../src/logic/scheduler.cpp
    ../src/logic/interval_revelator.cpp
)

add_executable(blaspheme ${SOURCES})
target_link_libraries(blaspheme Vulkan::Vulkan)
target_link_libraries(blaspheme glfw)
