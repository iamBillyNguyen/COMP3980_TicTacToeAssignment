cmake_minimum_required(VERSION 3.17)
project(COMP3980_TicTacToeAssignment C)

set(CMAKE_C_STANDARD 11)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(server server.c modules/server_utils.c modules/utils.c environment/TTTGame.c environment/RPSGame.c)
target_link_libraries(server dcfsm dc)
target_compile_options(server PRIVATE -Wpedantic -Wall -Wextra -ldc)

