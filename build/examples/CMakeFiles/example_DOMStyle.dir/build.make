# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/d/cs_git_rep/project/goahead-json

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/d/cs_git_rep/project/goahead-json/build

# Include any dependencies generated for this target.
include examples/CMakeFiles/example_DOMStyle.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/example_DOMStyle.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/example_DOMStyle.dir/flags.make

examples/CMakeFiles/example_DOMStyle.dir/example_DOMStyle.cc.o: examples/CMakeFiles/example_DOMStyle.dir/flags.make
examples/CMakeFiles/example_DOMStyle.dir/example_DOMStyle.cc.o: ../examples/example_DOMStyle.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/d/cs_git_rep/project/goahead-json/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/CMakeFiles/example_DOMStyle.dir/example_DOMStyle.cc.o"
	cd /mnt/d/cs_git_rep/project/goahead-json/build/examples && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/example_DOMStyle.dir/example_DOMStyle.cc.o -c /mnt/d/cs_git_rep/project/goahead-json/examples/example_DOMStyle.cc

examples/CMakeFiles/example_DOMStyle.dir/example_DOMStyle.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/example_DOMStyle.dir/example_DOMStyle.cc.i"
	cd /mnt/d/cs_git_rep/project/goahead-json/build/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/d/cs_git_rep/project/goahead-json/examples/example_DOMStyle.cc > CMakeFiles/example_DOMStyle.dir/example_DOMStyle.cc.i

examples/CMakeFiles/example_DOMStyle.dir/example_DOMStyle.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/example_DOMStyle.dir/example_DOMStyle.cc.s"
	cd /mnt/d/cs_git_rep/project/goahead-json/build/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/d/cs_git_rep/project/goahead-json/examples/example_DOMStyle.cc -o CMakeFiles/example_DOMStyle.dir/example_DOMStyle.cc.s

# Object files for target example_DOMStyle
example_DOMStyle_OBJECTS = \
"CMakeFiles/example_DOMStyle.dir/example_DOMStyle.cc.o"

# External object files for target example_DOMStyle
example_DOMStyle_EXTERNAL_OBJECTS =

bin/example_DOMStyle: examples/CMakeFiles/example_DOMStyle.dir/example_DOMStyle.cc.o
bin/example_DOMStyle: examples/CMakeFiles/example_DOMStyle.dir/build.make
bin/example_DOMStyle: lib/libgoa-json.a
bin/example_DOMStyle: examples/CMakeFiles/example_DOMStyle.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/d/cs_git_rep/project/goahead-json/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/example_DOMStyle"
	cd /mnt/d/cs_git_rep/project/goahead-json/build/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/example_DOMStyle.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/example_DOMStyle.dir/build: bin/example_DOMStyle

.PHONY : examples/CMakeFiles/example_DOMStyle.dir/build

examples/CMakeFiles/example_DOMStyle.dir/clean:
	cd /mnt/d/cs_git_rep/project/goahead-json/build/examples && $(CMAKE_COMMAND) -P CMakeFiles/example_DOMStyle.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/example_DOMStyle.dir/clean

examples/CMakeFiles/example_DOMStyle.dir/depend:
	cd /mnt/d/cs_git_rep/project/goahead-json/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/d/cs_git_rep/project/goahead-json /mnt/d/cs_git_rep/project/goahead-json/examples /mnt/d/cs_git_rep/project/goahead-json/build /mnt/d/cs_git_rep/project/goahead-json/build/examples /mnt/d/cs_git_rep/project/goahead-json/build/examples/CMakeFiles/example_DOMStyle.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/example_DOMStyle.dir/depend
