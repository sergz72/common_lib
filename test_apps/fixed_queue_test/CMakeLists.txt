cmake_minimum_required(VERSION 4.0)
project(fixed_queue_test C)

set(CMAKE_C_STANDARD 23)

add_executable(fixed_queue_test main.c ../../my/Programs/ARM_CLION/common_lib/fixed_queue.c)

include_directories(../../my/Programs/ARM_CLION/common_lib)