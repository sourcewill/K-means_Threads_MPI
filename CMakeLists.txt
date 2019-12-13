cmake_minimum_required(VERSION 3.13)
project(simple_example)
set(CMAKE_CXX_STANDARD 14)

add_executable(Paralelo paralelo.c)
add_executable(Sequencial sequencial.c)
add_executable(MPI MPI.c)