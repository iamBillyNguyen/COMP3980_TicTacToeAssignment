# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/billy/Downloads/clion-2020.2.1/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/billy/Downloads/clion-2020.2.1/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/billy/CLionProjects/COMP3980_TicTacToeAssignment

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/server.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/server.dir/flags.make

CMakeFiles/server.dir/server.c.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/server.c.o: ../server.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/billy/CLionProjects/COMP3980_TicTacToeAssignment/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/server.dir/server.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/server.dir/server.c.o   -c /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/server.c

CMakeFiles/server.dir/server.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/server.dir/server.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/server.c > CMakeFiles/server.dir/server.c.i

CMakeFiles/server.dir/server.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/server.dir/server.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/server.c -o CMakeFiles/server.dir/server.c.s
<<<<<<< HEAD
=======

CMakeFiles/server.dir/modules/server_utils.c.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/modules/server_utils.c.o: ../modules/server_utils.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/billy/CLionProjects/COMP3980_TicTacToeAssignment/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/server.dir/modules/server_utils.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/server.dir/modules/server_utils.c.o   -c /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/modules/server_utils.c

CMakeFiles/server.dir/modules/server_utils.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/server.dir/modules/server_utils.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/modules/server_utils.c > CMakeFiles/server.dir/modules/server_utils.c.i

CMakeFiles/server.dir/modules/server_utils.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/server.dir/modules/server_utils.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/modules/server_utils.c -o CMakeFiles/server.dir/modules/server_utils.c.s

CMakeFiles/server.dir/modules/utils.c.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/modules/utils.c.o: ../modules/utils.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/billy/CLionProjects/COMP3980_TicTacToeAssignment/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/server.dir/modules/utils.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/server.dir/modules/utils.c.o   -c /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/modules/utils.c

CMakeFiles/server.dir/modules/utils.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/server.dir/modules/utils.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/modules/utils.c > CMakeFiles/server.dir/modules/utils.c.i

CMakeFiles/server.dir/modules/utils.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/server.dir/modules/utils.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/modules/utils.c -o CMakeFiles/server.dir/modules/utils.c.s

CMakeFiles/server.dir/environment/TTTGame.c.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/environment/TTTGame.c.o: ../environment/TTTGame.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/billy/CLionProjects/COMP3980_TicTacToeAssignment/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/server.dir/environment/TTTGame.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/server.dir/environment/TTTGame.c.o   -c /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/environment/TTTGame.c

CMakeFiles/server.dir/environment/TTTGame.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/server.dir/environment/TTTGame.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/environment/TTTGame.c > CMakeFiles/server.dir/environment/TTTGame.c.i

CMakeFiles/server.dir/environment/TTTGame.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/server.dir/environment/TTTGame.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/environment/TTTGame.c -o CMakeFiles/server.dir/environment/TTTGame.c.s

CMakeFiles/server.dir/environment/RPSGame.c.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/environment/RPSGame.c.o: ../environment/RPSGame.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/billy/CLionProjects/COMP3980_TicTacToeAssignment/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/server.dir/environment/RPSGame.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/server.dir/environment/RPSGame.c.o   -c /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/environment/RPSGame.c

CMakeFiles/server.dir/environment/RPSGame.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/server.dir/environment/RPSGame.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/environment/RPSGame.c > CMakeFiles/server.dir/environment/RPSGame.c.i

CMakeFiles/server.dir/environment/RPSGame.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/server.dir/environment/RPSGame.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/environment/RPSGame.c -o CMakeFiles/server.dir/environment/RPSGame.c.s
>>>>>>> final_server

# Object files for target server
server_OBJECTS = \
"CMakeFiles/server.dir/server.c.o" \
"CMakeFiles/server.dir/modules/server_utils.c.o" \
"CMakeFiles/server.dir/modules/utils.c.o" \
"CMakeFiles/server.dir/environment/TTTGame.c.o" \
"CMakeFiles/server.dir/environment/RPSGame.c.o"

# External object files for target server
server_EXTERNAL_OBJECTS =

server: CMakeFiles/server.dir/server.c.o
server: CMakeFiles/server.dir/modules/server_utils.c.o
server: CMakeFiles/server.dir/modules/utils.c.o
server: CMakeFiles/server.dir/environment/TTTGame.c.o
server: CMakeFiles/server.dir/environment/RPSGame.c.o
server: CMakeFiles/server.dir/build.make
server: CMakeFiles/server.dir/link.txt
<<<<<<< HEAD
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/billy/CLionProjects/COMP3980_TicTacToeAssignment/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable server"
=======
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/billy/CLionProjects/COMP3980_TicTacToeAssignment/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking C executable server"
>>>>>>> final_server
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/server.dir/build: server

.PHONY : CMakeFiles/server.dir/build

CMakeFiles/server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/server.dir/clean

CMakeFiles/server.dir/depend:
	cd /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/billy/CLionProjects/COMP3980_TicTacToeAssignment /home/billy/CLionProjects/COMP3980_TicTacToeAssignment /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/cmake-build-debug /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/cmake-build-debug /home/billy/CLionProjects/COMP3980_TicTacToeAssignment/cmake-build-debug/CMakeFiles/server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/server.dir/depend

