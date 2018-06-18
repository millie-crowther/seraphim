cmake_minimum_required(VERSION 3.7)
set (CMAKE_CXX_STANDARD 11)
project (blaspheme)

#set(PATH_TO_GLFW /home/millie/glfw)

set(VULKAN_SDK_PATH "/home/millie/VulkanSDK/1.1.73.0/x86_64")

set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/include")
set(Vulkan_LIBRARY "${VULKAN_SDK_PATH}/lib/libvulkan.so")
find_package(Vulkan REQUIRED)


#set(CMAKE_EXE_LINKER_FLAGS "-L${VULKAN_SDK_PATH}/lib `pkg-config --static --libs glfw3` -lvulkan")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LDFLAGS}")


add_subdirectory("glfw")



set(INCLUDE_DIR
    ../include
)

include_directories(${INCLUDE_DIR})

set(SOURCES
    ../src/main.cpp
)

add_executable(blaspheme ${SOURCES})
target_link_libraries(blaspheme Vulkan::Vulkan)
target_link_libraries(blaspheme glfw)
