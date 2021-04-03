cmake_minimum_required(VERSION 3.7)
set (CMAKE_C_STANDARD  11)
project (seraphim_test)

SET(COMPILER_FLAGS "-fsanitize=address -Wall -Werror -Wfatal-errors")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")


set(INCLUDE_DIR
    .
    ../include
)

include_directories(${INCLUDE_DIR})

set(SOURCES
    test_main.c
)

add_executable(seraphim_test ${SOURCES})
