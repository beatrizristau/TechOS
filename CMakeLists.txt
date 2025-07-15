cmake_minimum_required(VERSION 3.20)
project(TechOS C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/include)

add_compile_definitions(HELP_DIR="${CMAKE_SOURCE_DIR}/help")

add_executable(TechOS src/main.c
        src/comhan.c
        include/comhan.h
        src/commands.c
        include/commands.h
        src/pcb.c
        include/pcb.h
        src/queue.c
        include/queue.h
        include/color_library.h
        src/utils.c
        include/utils.h
        src/dispatcher.c
        include/dispatcher.h
        src/auth.c
        include/auth.h)
