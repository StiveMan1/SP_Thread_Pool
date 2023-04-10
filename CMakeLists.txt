cmake_minimum_required(VERSION 3.24)
project(SP_Thread_Pool C)

set(GCC_COVERAGE_COMPILE_FLAGS "-pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

set(CMAKE_C_STANDARD 11)

add_executable(SP_Thread_Pool main.c thread_pool.c)
