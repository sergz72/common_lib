cmake_minimum_required(VERSION 3.29)
project(ObjectLoader C)

set(CMAKE_C_STANDARD 11)

add_executable(ObjectLoader main.c object_file_loader.c)
