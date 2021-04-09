# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

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

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/steve/libs/tatooine2/flowexplorer/external/imgui-node-editor/examples

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/steve/libs/tatooine2/build_imgui_node_editor

# Include any dependencies generated for this target.
include simple-example/CMakeFiles/simple-example.dir/depend.make

# Include the progress variables for this target.
include simple-example/CMakeFiles/simple-example.dir/progress.make

# Include the compile flags for this target's objects.
include simple-example/CMakeFiles/simple-example.dir/flags.make

simple-example/CMakeFiles/simple-example.dir/simple-example.cpp.o: simple-example/CMakeFiles/simple-example.dir/flags.make
simple-example/CMakeFiles/simple-example.dir/simple-example.cpp.o: /home/steve/libs/tatooine2/flowexplorer/external/imgui-node-editor/examples/simple-example/simple-example.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/steve/libs/tatooine2/build_imgui_node_editor/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object simple-example/CMakeFiles/simple-example.dir/simple-example.cpp.o"
	cd /home/steve/libs/tatooine2/build_imgui_node_editor/simple-example && /usr/lib/ccache/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/simple-example.dir/simple-example.cpp.o -c /home/steve/libs/tatooine2/flowexplorer/external/imgui-node-editor/examples/simple-example/simple-example.cpp

simple-example/CMakeFiles/simple-example.dir/simple-example.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simple-example.dir/simple-example.cpp.i"
	cd /home/steve/libs/tatooine2/build_imgui_node_editor/simple-example && /usr/lib/ccache/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/steve/libs/tatooine2/flowexplorer/external/imgui-node-editor/examples/simple-example/simple-example.cpp > CMakeFiles/simple-example.dir/simple-example.cpp.i

simple-example/CMakeFiles/simple-example.dir/simple-example.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simple-example.dir/simple-example.cpp.s"
	cd /home/steve/libs/tatooine2/build_imgui_node_editor/simple-example && /usr/lib/ccache/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/steve/libs/tatooine2/flowexplorer/external/imgui-node-editor/examples/simple-example/simple-example.cpp -o CMakeFiles/simple-example.dir/simple-example.cpp.s

# Object files for target simple-example
simple__example_OBJECTS = \
"CMakeFiles/simple-example.dir/simple-example.cpp.o"

# External object files for target simple-example
simple__example_EXTERNAL_OBJECTS =

bin/simple-example_d: simple-example/CMakeFiles/simple-example.dir/simple-example.cpp.o
bin/simple-example_d: simple-example/CMakeFiles/simple-example.dir/build.make
bin/simple-example_d: external/imgui/libimgui.a
bin/simple-example_d: canvas-example/libimgui_node_editor.a
bin/simple-example_d: application/libapplication.a
bin/simple-example_d: external/imgui/libimgui.a
bin/simple-example_d: external/gl3w/libgl3w.a
bin/simple-example_d: /usr/lib/libglfw.so.3.3
bin/simple-example_d: simple-example/CMakeFiles/simple-example.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/steve/libs/tatooine2/build_imgui_node_editor/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/simple-example_d"
	cd /home/steve/libs/tatooine2/build_imgui_node_editor/simple-example && /usr/bin/cmake -E make_directory /home/steve/libs/tatooine2/build_imgui_node_editor/bin/data
	cd /home/steve/libs/tatooine2/build_imgui_node_editor/simple-example && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simple-example.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
simple-example/CMakeFiles/simple-example.dir/build: bin/simple-example_d

.PHONY : simple-example/CMakeFiles/simple-example.dir/build

simple-example/CMakeFiles/simple-example.dir/clean:
	cd /home/steve/libs/tatooine2/build_imgui_node_editor/simple-example && $(CMAKE_COMMAND) -P CMakeFiles/simple-example.dir/cmake_clean.cmake
.PHONY : simple-example/CMakeFiles/simple-example.dir/clean

simple-example/CMakeFiles/simple-example.dir/depend:
	cd /home/steve/libs/tatooine2/build_imgui_node_editor && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/steve/libs/tatooine2/flowexplorer/external/imgui-node-editor/examples /home/steve/libs/tatooine2/flowexplorer/external/imgui-node-editor/examples/simple-example /home/steve/libs/tatooine2/build_imgui_node_editor /home/steve/libs/tatooine2/build_imgui_node_editor/simple-example /home/steve/libs/tatooine2/build_imgui_node_editor/simple-example/CMakeFiles/simple-example.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : simple-example/CMakeFiles/simple-example.dir/depend
