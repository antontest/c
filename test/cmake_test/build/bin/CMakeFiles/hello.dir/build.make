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
CMAKE_SOURCE_DIR = /home/anton/working/program/c/cmake_test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/anton/working/program/c/cmake_test/build

# Include any dependencies generated for this target.
include bin/CMakeFiles/hello.dir/depend.make

# Include the progress variables for this target.
include bin/CMakeFiles/hello.dir/progress.make

# Include the compile flags for this target's objects.
include bin/CMakeFiles/hello.dir/flags.make

bin/CMakeFiles/hello.dir/hello/hello.c.o: bin/CMakeFiles/hello.dir/flags.make
bin/CMakeFiles/hello.dir/hello/hello.c.o: ../src/hello/hello.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/anton/working/program/c/cmake_test/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object bin/CMakeFiles/hello.dir/hello/hello.c.o"
	cd /home/anton/working/program/c/cmake_test/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/hello.dir/hello/hello.c.o   -c /home/anton/working/program/c/cmake_test/src/hello/hello.c

bin/CMakeFiles/hello.dir/hello/hello.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/hello.dir/hello/hello.c.i"
	cd /home/anton/working/program/c/cmake_test/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/anton/working/program/c/cmake_test/src/hello/hello.c > CMakeFiles/hello.dir/hello/hello.c.i

bin/CMakeFiles/hello.dir/hello/hello.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/hello.dir/hello/hello.c.s"
	cd /home/anton/working/program/c/cmake_test/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/anton/working/program/c/cmake_test/src/hello/hello.c -o CMakeFiles/hello.dir/hello/hello.c.s

bin/CMakeFiles/hello.dir/hello/hello.c.o.requires:
.PHONY : bin/CMakeFiles/hello.dir/hello/hello.c.o.requires

bin/CMakeFiles/hello.dir/hello/hello.c.o.provides: bin/CMakeFiles/hello.dir/hello/hello.c.o.requires
	$(MAKE) -f bin/CMakeFiles/hello.dir/build.make bin/CMakeFiles/hello.dir/hello/hello.c.o.provides.build
.PHONY : bin/CMakeFiles/hello.dir/hello/hello.c.o.provides

bin/CMakeFiles/hello.dir/hello/hello.c.o.provides.build: bin/CMakeFiles/hello.dir/hello/hello.c.o

# Object files for target hello
hello_OBJECTS = \
"CMakeFiles/hello.dir/hello/hello.c.o"

# External object files for target hello
hello_EXTERNAL_OBJECTS =

../src/lib/libhello.a: bin/CMakeFiles/hello.dir/hello/hello.c.o
../src/lib/libhello.a: bin/CMakeFiles/hello.dir/build.make
../src/lib/libhello.a: bin/CMakeFiles/hello.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C static library ../../src/lib/libhello.a"
	cd /home/anton/working/program/c/cmake_test/build/bin && $(CMAKE_COMMAND) -P CMakeFiles/hello.dir/cmake_clean_target.cmake
	cd /home/anton/working/program/c/cmake_test/build/bin && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/hello.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bin/CMakeFiles/hello.dir/build: ../src/lib/libhello.a
.PHONY : bin/CMakeFiles/hello.dir/build

bin/CMakeFiles/hello.dir/requires: bin/CMakeFiles/hello.dir/hello/hello.c.o.requires
.PHONY : bin/CMakeFiles/hello.dir/requires

bin/CMakeFiles/hello.dir/clean:
	cd /home/anton/working/program/c/cmake_test/build/bin && $(CMAKE_COMMAND) -P CMakeFiles/hello.dir/cmake_clean.cmake
.PHONY : bin/CMakeFiles/hello.dir/clean

bin/CMakeFiles/hello.dir/depend:
	cd /home/anton/working/program/c/cmake_test/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/anton/working/program/c/cmake_test /home/anton/working/program/c/cmake_test/src /home/anton/working/program/c/cmake_test/build /home/anton/working/program/c/cmake_test/build/bin /home/anton/working/program/c/cmake_test/build/bin/CMakeFiles/hello.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bin/CMakeFiles/hello.dir/depend

