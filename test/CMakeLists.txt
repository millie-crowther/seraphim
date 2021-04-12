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
    ../src/maths/sdf/sdf.cpp
    ../src/maths/sdf/primitive.cpp
    ../src/maths/sdf/platonic.cpp
        ../src/maths/bound.cpp
    ../src/core/random.cpp
    ../src/physics/transform.cpp
    test_main.cpp
)

add_executable(seraphim_test ${SOURCES})
