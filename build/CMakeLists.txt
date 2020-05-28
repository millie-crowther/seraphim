cmake_minimum_required(VERSION 3.7)
set (CMAKE_CXX_STANDARD 17)
project (seraphim)

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
)

include_directories(${INCLUDE_DIR})

set(SOURCES
    ../src/main.cpp
  
    ../src/core/seraphim.cpp
    ../src/core/command.cpp
    ../src/core/uuid.cpp
    ../src/core/device.cpp
    
    ../src/physics/physics.cpp

    ../src/render/renderer.cpp
    ../src/render/texture.cpp
    ../src/render/camera.cpp
    ../src/render/swapchain.cpp
    ../src/render/call_and_response.cpp

    ../src/substance/substance.cpp
    ../src/substance/form/form.cpp
    ../src/substance/matter/matter.cpp
    
    ../src/maths/quat.cpp
    
    ../src/scene/transform.cpp

    ../src/ui/keyboard.cpp
    ../src/ui/resources.cpp
    ../src/ui/window.cpp
    ../src/ui/mouse.cpp

    ../src/logic/scheduler.cpp
)

add_executable(seraphim ${SOURCES})
target_link_libraries(seraphim Vulkan::Vulkan)
target_link_libraries(seraphim glfw)
