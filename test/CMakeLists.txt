cmake_minimum_required(VERSION 3.7)
set (CMAKE_CXX_STANDARD 17)
project (seraphim_test)

SET(COMPILER_FLAGS "-fsanitize=address -Wall -Werror -Wfatal-errors")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")


set(INCLUDE_DIR
    .
    ../include
)

include_directories(${INCLUDE_DIR})

set(SOURCES
        ../backend/sdf.cpp
        ../backend/primitive.cpp
        ../backend/platonic.cpp
        ../common/bound.cpp
        ../common/random.cpp
        ../common/transform.cpp
        ../common/array.cpp
        ../common/maths.cpp
        ../common/matter.cpp
    test_main.cpp
)

add_executable(seraphim_test ${SOURCES})
