# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.1

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/anton/program/c/cmake/socket

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/anton/program/c/cmake/socket/build

# Include any dependencies generated for this target.
include bin/CMakeFiles/socket.dir/depend.make

# Include the progress variables for this target.
include bin/CMakeFiles/socket.dir/progress.make

# Include the compile flags for this target's objects.
include bin/CMakeFiles/socket.dir/flags.make

bin/CMakeFiles/socket.dir/main/socket.c.o: bin/CMakeFiles/socket.dir/flags.make
bin/CMakeFiles/socket.dir/main/socket.c.o: ../src/main/socket.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/anton/program/c/cmake/socket/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object bin/CMakeFiles/socket.dir/main/socket.c.o"
	cd /home/anton/program/c/cmake/socket/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/socket.dir/main/socket.c.o   -c /home/anton/program/c/cmake/socket/src/main/socket.c

bin/CMakeFiles/socket.dir/main/socket.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/socket.dir/main/socket.c.i"
	cd /home/anton/program/c/cmake/socket/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/anton/program/c/cmake/socket/src/main/socket.c > CMakeFiles/socket.dir/main/socket.c.i

bin/CMakeFiles/socket.dir/main/socket.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/socket.dir/main/socket.c.s"
	cd /home/anton/program/c/cmake/socket/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/anton/program/c/cmake/socket/src/main/socket.c -o CMakeFiles/socket.dir/main/socket.c.s

bin/CMakeFiles/socket.dir/main/socket.c.o.requires:
.PHONY : bin/CMakeFiles/socket.dir/main/socket.c.o.requires

bin/CMakeFiles/socket.dir/main/socket.c.o.provides: bin/CMakeFiles/socket.dir/main/socket.c.o.requires
	$(MAKE) -f bin/CMakeFiles/socket.dir/build.make bin/CMakeFiles/socket.dir/main/socket.c.o.provides.build
.PHONY : bin/CMakeFiles/socket.dir/main/socket.c.o.provides

bin/CMakeFiles/socket.dir/main/socket.c.o.provides.build: bin/CMakeFiles/socket.dir/main/socket.c.o

# Object files for target socket
socket_OBJECTS = \
"CMakeFiles/socket.dir/main/socket.c.o"

# External object files for target socket
socket_EXTERNAL_OBJECTS =

../socket: bin/CMakeFiles/socket.dir/main/socket.c.o
../socket: bin/CMakeFiles/socket.dir/build.make
../socket: ../src/lib/libsocket_app.so
../socket: ../src/lib/libsocket_arp.so
../socket: ../src/lib/libsocket_base.a
../socket: ../src/lib/libsocket_property.a
../socket: bin/CMakeFiles/socket.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable ../../socket"
	cd /home/anton/program/c/cmake/socket/build/bin && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/socket.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bin/CMakeFiles/socket.dir/build: ../socket
.PHONY : bin/CMakeFiles/socket.dir/build

bin/CMakeFiles/socket.dir/requires: bin/CMakeFiles/socket.dir/main/socket.c.o.requires
.PHONY : bin/CMakeFiles/socket.dir/requires

bin/CMakeFiles/socket.dir/clean:
	cd /home/anton/program/c/cmake/socket/build/bin && $(CMAKE_COMMAND) -P CMakeFiles/socket.dir/cmake_clean.cmake
.PHONY : bin/CMakeFiles/socket.dir/clean

bin/CMakeFiles/socket.dir/depend:
	cd /home/anton/program/c/cmake/socket/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/anton/program/c/cmake/socket /home/anton/program/c/cmake/socket/src /home/anton/program/c/cmake/socket/build /home/anton/program/c/cmake/socket/build/bin /home/anton/program/c/cmake/socket/build/bin/CMakeFiles/socket.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bin/CMakeFiles/socket.dir/depend

