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
CMAKE_SOURCE_DIR = /home/anton/program/lib/queue

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/anton/program/lib/queue/build

# Include any dependencies generated for this target.
include bin/CMakeFiles/queue.dir/depend.make

# Include the progress variables for this target.
include bin/CMakeFiles/queue.dir/progress.make

# Include the compile flags for this target's objects.
include bin/CMakeFiles/queue.dir/flags.make

bin/CMakeFiles/queue.dir/main/queue.c.o: bin/CMakeFiles/queue.dir/flags.make
bin/CMakeFiles/queue.dir/main/queue.c.o: ../src/main/queue.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/anton/program/lib/queue/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object bin/CMakeFiles/queue.dir/main/queue.c.o"
	cd /home/anton/program/lib/queue/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/queue.dir/main/queue.c.o   -c /home/anton/program/lib/queue/src/main/queue.c

bin/CMakeFiles/queue.dir/main/queue.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/queue.dir/main/queue.c.i"
	cd /home/anton/program/lib/queue/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/anton/program/lib/queue/src/main/queue.c > CMakeFiles/queue.dir/main/queue.c.i

bin/CMakeFiles/queue.dir/main/queue.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/queue.dir/main/queue.c.s"
	cd /home/anton/program/lib/queue/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/anton/program/lib/queue/src/main/queue.c -o CMakeFiles/queue.dir/main/queue.c.s

bin/CMakeFiles/queue.dir/main/queue.c.o.requires:
.PHONY : bin/CMakeFiles/queue.dir/main/queue.c.o.requires

bin/CMakeFiles/queue.dir/main/queue.c.o.provides: bin/CMakeFiles/queue.dir/main/queue.c.o.requires
	$(MAKE) -f bin/CMakeFiles/queue.dir/build.make bin/CMakeFiles/queue.dir/main/queue.c.o.provides.build
.PHONY : bin/CMakeFiles/queue.dir/main/queue.c.o.provides

bin/CMakeFiles/queue.dir/main/queue.c.o.provides.build: bin/CMakeFiles/queue.dir/main/queue.c.o

# Object files for target queue
queue_OBJECTS = \
"CMakeFiles/queue.dir/main/queue.c.o"

# External object files for target queue
queue_EXTERNAL_OBJECTS =

../src/lib/libqueue.a: bin/CMakeFiles/queue.dir/main/queue.c.o
../src/lib/libqueue.a: bin/CMakeFiles/queue.dir/build.make
../src/lib/libqueue.a: bin/CMakeFiles/queue.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C static library ../../src/lib/libqueue.a"
	cd /home/anton/program/lib/queue/build/bin && $(CMAKE_COMMAND) -P CMakeFiles/queue.dir/cmake_clean_target.cmake
	cd /home/anton/program/lib/queue/build/bin && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/queue.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bin/CMakeFiles/queue.dir/build: ../src/lib/libqueue.a
.PHONY : bin/CMakeFiles/queue.dir/build

bin/CMakeFiles/queue.dir/requires: bin/CMakeFiles/queue.dir/main/queue.c.o.requires
.PHONY : bin/CMakeFiles/queue.dir/requires

bin/CMakeFiles/queue.dir/clean:
	cd /home/anton/program/lib/queue/build/bin && $(CMAKE_COMMAND) -P CMakeFiles/queue.dir/cmake_clean.cmake
.PHONY : bin/CMakeFiles/queue.dir/clean

bin/CMakeFiles/queue.dir/depend:
	cd /home/anton/program/lib/queue/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/anton/program/lib/queue /home/anton/program/lib/queue/src /home/anton/program/lib/queue/build /home/anton/program/lib/queue/build/bin /home/anton/program/lib/queue/build/bin/CMakeFiles/queue.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bin/CMakeFiles/queue.dir/depend

