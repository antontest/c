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
CMAKE_SOURCE_DIR = /home/anton/program/c/cmake/print

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/anton/program/c/cmake/print/build

# Include any dependencies generated for this target.
include bin/CMakeFiles/print_static.dir/depend.make

# Include the progress variables for this target.
include bin/CMakeFiles/print_static.dir/progress.make

# Include the compile flags for this target's objects.
include bin/CMakeFiles/print_static.dir/flags.make

bin/CMakeFiles/print_static.dir/print/print.c.o: bin/CMakeFiles/print_static.dir/flags.make
bin/CMakeFiles/print_static.dir/print/print.c.o: ../src/print/print.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/anton/program/c/cmake/print/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object bin/CMakeFiles/print_static.dir/print/print.c.o"
	cd /home/anton/program/c/cmake/print/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/print_static.dir/print/print.c.o   -c /home/anton/program/c/cmake/print/src/print/print.c

bin/CMakeFiles/print_static.dir/print/print.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/print_static.dir/print/print.c.i"
	cd /home/anton/program/c/cmake/print/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/anton/program/c/cmake/print/src/print/print.c > CMakeFiles/print_static.dir/print/print.c.i

bin/CMakeFiles/print_static.dir/print/print.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/print_static.dir/print/print.c.s"
	cd /home/anton/program/c/cmake/print/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/anton/program/c/cmake/print/src/print/print.c -o CMakeFiles/print_static.dir/print/print.c.s

bin/CMakeFiles/print_static.dir/print/print.c.o.requires:
.PHONY : bin/CMakeFiles/print_static.dir/print/print.c.o.requires

bin/CMakeFiles/print_static.dir/print/print.c.o.provides: bin/CMakeFiles/print_static.dir/print/print.c.o.requires
	$(MAKE) -f bin/CMakeFiles/print_static.dir/build.make bin/CMakeFiles/print_static.dir/print/print.c.o.provides.build
.PHONY : bin/CMakeFiles/print_static.dir/print/print.c.o.provides

bin/CMakeFiles/print_static.dir/print/print.c.o.provides.build: bin/CMakeFiles/print_static.dir/print/print.c.o

# Object files for target print_static
print_static_OBJECTS = \
"CMakeFiles/print_static.dir/print/print.c.o"

# External object files for target print_static
print_static_EXTERNAL_OBJECTS =

../src/lib/libprint.a: bin/CMakeFiles/print_static.dir/print/print.c.o
../src/lib/libprint.a: bin/CMakeFiles/print_static.dir/build.make
../src/lib/libprint.a: bin/CMakeFiles/print_static.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C static library ../../src/lib/libprint.a"
	cd /home/anton/program/c/cmake/print/build/bin && $(CMAKE_COMMAND) -P CMakeFiles/print_static.dir/cmake_clean_target.cmake
	cd /home/anton/program/c/cmake/print/build/bin && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/print_static.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bin/CMakeFiles/print_static.dir/build: ../src/lib/libprint.a
.PHONY : bin/CMakeFiles/print_static.dir/build

bin/CMakeFiles/print_static.dir/requires: bin/CMakeFiles/print_static.dir/print/print.c.o.requires
.PHONY : bin/CMakeFiles/print_static.dir/requires

bin/CMakeFiles/print_static.dir/clean:
	cd /home/anton/program/c/cmake/print/build/bin && $(CMAKE_COMMAND) -P CMakeFiles/print_static.dir/cmake_clean.cmake
.PHONY : bin/CMakeFiles/print_static.dir/clean

bin/CMakeFiles/print_static.dir/depend:
	cd /home/anton/program/c/cmake/print/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/anton/program/c/cmake/print /home/anton/program/c/cmake/print/src /home/anton/program/c/cmake/print/build /home/anton/program/c/cmake/print/build/bin /home/anton/program/c/cmake/print/build/bin/CMakeFiles/print_static.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bin/CMakeFiles/print_static.dir/depend

