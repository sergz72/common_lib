cmake_minimum_required(VERSION 4.3)
project(ElfLoader C)

set(CMAKE_C_STANDARD 23)

add_executable(ElfLoader main.c ../../my/Programs/common_lib/elf_file_loader.c)

include_directories(. ../../my/Programs/common_lib ../../my/Programs/RISCV/GD32VW55x/szos_gd23vw553/syscalls/x64)
