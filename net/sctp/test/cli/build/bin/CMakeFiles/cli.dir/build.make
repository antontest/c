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
CMAKE_SOURCE_DIR = /home/anton/program/c/sctp/test/cli

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/anton/program/c/sctp/test/cli/build

# Include any dependencies generated for this target.
include bin/CMakeFiles/cli.dir/depend.make

# Include the progress variables for this target.
include bin/CMakeFiles/cli.dir/progress.make

# Include the compile flags for this target's objects.
include bin/CMakeFiles/cli.dir/flags.make

bin/CMakeFiles/cli.dir/main/cli.c.o: bin/CMakeFiles/cli.dir/flags.make
bin/CMakeFiles/cli.dir/main/cli.c.o: ../src/main/cli.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/anton/program/c/sctp/test/cli/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object bin/CMakeFiles/cli.dir/main/cli.c.o"
	cd /home/anton/program/c/sctp/test/cli/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/cli.dir/main/cli.c.o   -c /home/anton/program/c/sctp/test/cli/src/main/cli.c

bin/CMakeFiles/cli.dir/main/cli.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cli.dir/main/cli.c.i"
	cd /home/anton/program/c/sctp/test/cli/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/anton/program/c/sctp/test/cli/src/main/cli.c > CMakeFiles/cli.dir/main/cli.c.i

bin/CMakeFiles/cli.dir/main/cli.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cli.dir/main/cli.c.s"
	cd /home/anton/program/c/sctp/test/cli/build/bin && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/anton/program/c/sctp/test/cli/src/main/cli.c -o CMakeFiles/cli.dir/main/cli.c.s

bin/CMakeFiles/cli.dir/main/cli.c.o.requires:
.PHONY : bin/CMakeFiles/cli.dir/main/cli.c.o.requires

bin/CMakeFiles/cli.dir/main/cli.c.o.provides: bin/CMakeFiles/cli.dir/main/cli.c.o.requires
	$(MAKE) -f bin/CMakeFiles/cli.dir/build.make bin/CMakeFiles/cli.dir/main/cli.c.o.provides.build
.PHONY : bin/CMakeFiles/cli.dir/main/cli.c.o.provides

bin/CMakeFiles/cli.dir/main/cli.c.o.provides.build: bin/CMakeFiles/cli.dir/main/cli.c.o

# Object files for target cli
cli_OBJECTS = \
"CMakeFiles/cli.dir/main/cli.c.o"

# External object files for target cli
cli_EXTERNAL_OBJECTS =

../cli: bin/CMakeFiles/cli.dir/main/cli.c.o
../cli: bin/CMakeFiles/cli.dir/build.make
../cli: bin/CMakeFiles/cli.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable ../../cli"
	cd /home/anton/program/c/sctp/test/cli/build/bin && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cli.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bin/CMakeFiles/cli.dir/build: ../cli
.PHONY : bin/CMakeFiles/cli.dir/build

bin/CMakeFiles/cli.dir/requires: bin/CMakeFiles/cli.dir/main/cli.c.o.requires
.PHONY : bin/CMakeFiles/cli.dir/requires

bin/CMakeFiles/cli.dir/clean:
	cd /home/anton/program/c/sctp/test/cli/build/bin && $(CMAKE_COMMAND) -P CMakeFiles/cli.dir/cmake_clean.cmake
.PHONY : bin/CMakeFiles/cli.dir/clean

bin/CMakeFiles/cli.dir/depend:
	cd /home/anton/program/c/sctp/test/cli/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/anton/program/c/sctp/test/cli /home/anton/program/c/sctp/test/cli/src /home/anton/program/c/sctp/test/cli/build /home/anton/program/c/sctp/test/cli/build/bin /home/anton/program/c/sctp/test/cli/build/bin/CMakeFiles/cli.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bin/CMakeFiles/cli.dir/depend

