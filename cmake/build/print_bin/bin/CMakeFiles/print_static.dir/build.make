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
CMAKE_SOURCE_DIR = /home/anton/program/c/cmake

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/anton/program/c/cmake/build

# Include any dependencies generated for this target.
include print_bin/bin/CMakeFiles/print_static.dir/depend.make

# Include the progress variables for this target.
include print_bin/bin/CMakeFiles/print_static.dir/progress.make

# Include the compile flags for this target's objects.
include print_bin/bin/CMakeFiles/print_static.dir/flags.make

print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o: print_bin/bin/CMakeFiles/print_static.dir/flags.make
print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o: ../print/src/print/print.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/anton/program/c/cmake/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o"
	cd /home/anton/program/c/cmake/build/print_bin/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/print_static.dir/print/print.c.o   -c /home/anton/program/c/cmake/print/src/print/print.c

print_bin/bin/CMakeFiles/print_static.dir/print/print.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/print_static.dir/print/print.c.i"
	cd /home/anton/program/c/cmake/build/print_bin/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/anton/program/c/cmake/print/src/print/print.c > CMakeFiles/print_static.dir/print/print.c.i

print_bin/bin/CMakeFiles/print_static.dir/print/print.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/print_static.dir/print/print.c.s"
	cd /home/anton/program/c/cmake/build/print_bin/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/anton/program/c/cmake/print/src/print/print.c -o CMakeFiles/print_static.dir/print/print.c.s

print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o.requires:
.PHONY : print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o.requires

print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o.provides: print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o.requires
	$(MAKE) -f print_bin/bin/CMakeFiles/print_static.dir/build.make print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o.provides.build
.PHONY : print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o.provides

print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o.provides.build: print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o

# Object files for target print_static
print_static_OBJECTS = \
"CMakeFiles/print_static.dir/print/print.c.o"

# External object files for target print_static
print_static_EXTERNAL_OBJECTS =

../print/src/lib/libprint.a: print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o
../print/src/lib/libprint.a: print_bin/bin/CMakeFiles/print_static.dir/build.make
../print/src/lib/libprint.a: print_bin/bin/CMakeFiles/print_static.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C static library ../../../print/src/lib/libprint.a"
	cd /home/anton/program/c/cmake/build/print_bin/bin && $(CMAKE_COMMAND) -P CMakeFiles/print_static.dir/cmake_clean_target.cmake
	cd /home/anton/program/c/cmake/build/print_bin/bin && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/print_static.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
print_bin/bin/CMakeFiles/print_static.dir/build: ../print/src/lib/libprint.a
.PHONY : print_bin/bin/CMakeFiles/print_static.dir/build

print_bin/bin/CMakeFiles/print_static.dir/requires: print_bin/bin/CMakeFiles/print_static.dir/print/print.c.o.requires
.PHONY : print_bin/bin/CMakeFiles/print_static.dir/requires

print_bin/bin/CMakeFiles/print_static.dir/clean:
	cd /home/anton/program/c/cmake/build/print_bin/bin && $(CMAKE_COMMAND) -P CMakeFiles/print_static.dir/cmake_clean.cmake
.PHONY : print_bin/bin/CMakeFiles/print_static.dir/clean

print_bin/bin/CMakeFiles/print_static.dir/depend:
	cd /home/anton/program/c/cmake/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/anton/program/c/cmake /home/anton/program/c/cmake/print/src /home/anton/program/c/cmake/build /home/anton/program/c/cmake/build/print_bin/bin /home/anton/program/c/cmake/build/print_bin/bin/CMakeFiles/print_static.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : print_bin/bin/CMakeFiles/print_static.dir/depend

