cmake_minimum_required(VERSION 3.17)
project(COMP3980_Assignment1 C)

set(CMAKE_C_STANDARD 11)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(server serv.c)
target_link_libraries(server dcfsm dc)
target_compile_options(server PRIVATE -Wpedantic -Wall -Wextra -ldc)

