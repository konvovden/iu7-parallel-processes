

cmake_minimum_required(VERSION 3.29)
project(openmp C)

set(CMAKE_C_STANDARD 11)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")

add_executable(openmp
    Main.c)

find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
target_link_libraries(openmp ${MPI_C_LIBRARIES})
