cmake_minimum_required(VERSION 4.2)
project(rtc_func_test C)

set(CMAKE_C_STANDARD 23)

add_executable(rtc_func_test main.c ../../my/Programs/ARM_CLION/common_lib/rtc/rtc_func.c)

include_directories(../../my/Programs/ARM_CLION/common_lib/rtc)

