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
CMAKE_SOURCE_DIR = /home/anton/working/program/c/readini

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/anton/working/program/c/readini/build

# Include any dependencies generated for this target.
include bin/CMakeFiles/main.dir/depend.make

# Include the progress variables for this target.
include bin/CMakeFiles/main.dir/progress.make

# Include the compile flags for this target's objects.
include bin/CMakeFiles/main.dir/flags.make

bin/CMakeFiles/main.dir/main/readini.cpp.o: bin/CMakeFiles/main.dir/flags.make
bin/CMakeFiles/main.dir/main/readini.cpp.o: ../src/main/readini.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/anton/working/program/c/readini/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object bin/CMakeFiles/main.dir/main/readini.cpp.o"
	cd /home/anton/working/program/c/readini/build/bin && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/main.dir/main/readini.cpp.o -c /home/anton/working/program/c/readini/src/main/readini.cpp

bin/CMakeFiles/main.dir/main/readini.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/main.dir/main/readini.cpp.i"
	cd /home/anton/working/program/c/readini/build/bin && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/anton/working/program/c/readini/src/main/readini.cpp > CMakeFiles/main.dir/main/readini.cpp.i

bin/CMakeFiles/main.dir/main/readini.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/main.dir/main/readini.cpp.s"
	cd /home/anton/working/program/c/readini/build/bin && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/anton/working/program/c/readini/src/main/readini.cpp -o CMakeFiles/main.dir/main/readini.cpp.s

bin/CMakeFiles/main.dir/main/readini.cpp.o.requires:
.PHONY : bin/CMakeFiles/main.dir/main/readini.cpp.o.requires

bin/CMakeFiles/main.dir/main/readini.cpp.o.provides: bin/CMakeFiles/main.dir/main/readini.cpp.o.requires
	$(MAKE) -f bin/CMakeFiles/main.dir/build.make bin/CMakeFiles/main.dir/main/readini.cpp.o.provides.build
.PHONY : bin/CMakeFiles/main.dir/main/readini.cpp.o.provides

bin/CMakeFiles/main.dir/main/readini.cpp.o.provides.build: bin/CMakeFiles/main.dir/main/readini.cpp.o

# Object files for target main
main_OBJECTS = \
"CMakeFiles/main.dir/main/readini.cpp.o"

# External object files for target main
main_EXTERNAL_OBJECTS =

bin/main: bin/CMakeFiles/main.dir/main/readini.cpp.o
bin/main: bin/CMakeFiles/main.dir/build.make
bin/main: bin/CMakeFiles/main.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable main"
	cd /home/anton/working/program/c/readini/build/bin && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/main.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bin/CMakeFiles/main.dir/build: bin/main
.PHONY : bin/CMakeFiles/main.dir/build

bin/CMakeFiles/main.dir/requires: bin/CMakeFiles/main.dir/main/readini.cpp.o.requires
.PHONY : bin/CMakeFiles/main.dir/requires

bin/CMakeFiles/main.dir/clean:
	cd /home/anton/working/program/c/readini/build/bin && $(CMAKE_COMMAND) -P CMakeFiles/main.dir/cmake_clean.cmake
.PHONY : bin/CMakeFiles/main.dir/clean

bin/CMakeFiles/main.dir/depend:
	cd /home/anton/working/program/c/readini/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/anton/working/program/c/readini /home/anton/working/program/c/readini/src /home/anton/working/program/c/readini/build /home/anton/working/program/c/readini/build/bin /home/anton/working/program/c/readini/build/bin/CMakeFiles/main.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bin/CMakeFiles/main.dir/depend

