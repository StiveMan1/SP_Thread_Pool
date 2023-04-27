cmake_minimum_required(VERSION 3.25)
project(Assigmnent4 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(Assigmnent4
        main.c
        thread_pool.c
        thread_pool.h)
