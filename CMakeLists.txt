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

add_subdirectory("build/glfw")

set(INCLUDE_DIR
    include
    include/lib
    build/glfw/include
)

include_directories(${INCLUDE_DIR})

set(SOURCES
        common/main.cpp

        frontend/command.cpp
        frontend/device.cpp
        common/random.cpp
        common/seraphim.cpp
        common/array.cpp
        frontend/buffer.cpp

        common/cJSON.c

        backend/collision.cpp
        backend/physics.cpp
        common/sphere.cpp
        common/transform.cpp

        frontend/request.cpp
        common/camera.cpp
        common/light.cpp
        frontend/renderer.cpp
        frontend/swapchain.cpp
        frontend/texture.cpp
        frontend/shader.cpp

        backend/metaphysics.cpp
        common/material.cpp

        common/bound.cpp
        backend/optimise.cpp
        common/maths.cpp
        backend/sdf.cpp
        backend/primitive.cpp
        backend/platonic.cpp

        frontend/ui.cpp
        common/file.cpp
)

add_executable(seraphim ${SOURCES})
target_link_libraries(seraphim Vulkan::Vulkan)
target_link_libraries(seraphim glfw)
