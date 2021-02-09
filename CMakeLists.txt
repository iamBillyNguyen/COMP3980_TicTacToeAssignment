cmake_minimum_required(VERSION 3.17)
project(dcfsm C)

set(CMAKE_C_STANDARD 11)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(server server.c shared.h)
target_link_libraries(server dcfsm dc)
target_compile_options(server PRIVATE -Wpedantic -Wall -Wextra -ldc)

