# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.0

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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files (x86)\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files (x86)\CMake\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Project\Brain\SpatialIndex\cmake

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Project\Brain\SpatialIndex\build

# Include any dependencies generated for this target.
include CMakeFiles/TOUCH.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/TOUCH.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/TOUCH.dir/flags.make

CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj: CMakeFiles/TOUCH.dir/flags.make
CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj: CMakeFiles/TOUCH.dir/includes_CXX.rsp
CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj: C:/Project/Brain/SpatialIndex/apps/TOUCH.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report C:\Project\Brain\SpatialIndex\build\CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj"
	C:\MinGW\bin\g++.exe   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles\TOUCH.dir\C_\Project\Brain\SpatialIndex\apps\TOUCH.obj -c C:\Project\Brain\SpatialIndex\apps\TOUCH.cpp

CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.i"
	C:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_FLAGS) -E C:\Project\Brain\SpatialIndex\apps\TOUCH.cpp > CMakeFiles\TOUCH.dir\C_\Project\Brain\SpatialIndex\apps\TOUCH.i

CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.s"
	C:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_FLAGS) -S C:\Project\Brain\SpatialIndex\apps\TOUCH.cpp -o CMakeFiles\TOUCH.dir\C_\Project\Brain\SpatialIndex\apps\TOUCH.s

CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj.requires:
.PHONY : CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj.requires

CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj.provides: CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj.requires
	$(MAKE) -f CMakeFiles\TOUCH.dir\build.make CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj.provides.build
.PHONY : CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj.provides

CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj.provides.build: CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj

# Object files for target TOUCH
TOUCH_OBJECTS = \
"CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj"

# External object files for target TOUCH
TOUCH_EXTERNAL_OBJECTS =

C:/Project/Brain/SpatialIndex/bin/TOUCH.exe: CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj
C:/Project/Brain/SpatialIndex/bin/TOUCH.exe: CMakeFiles/TOUCH.dir/build.make
C:/Project/Brain/SpatialIndex/bin/TOUCH.exe: C:/Project/Brain/SpatialIndex/lib/libFLATIndex.a
C:/Project/Brain/SpatialIndex/bin/TOUCH.exe: CMakeFiles/TOUCH.dir/objects1.rsp
C:/Project/Brain/SpatialIndex/bin/TOUCH.exe: CMakeFiles/TOUCH.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable C:\Project\Brain\SpatialIndex\bin\TOUCH.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\TOUCH.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/TOUCH.dir/build: C:/Project/Brain/SpatialIndex/bin/TOUCH.exe
.PHONY : CMakeFiles/TOUCH.dir/build

CMakeFiles/TOUCH.dir/requires: CMakeFiles/TOUCH.dir/C_/Project/Brain/SpatialIndex/apps/TOUCH.obj.requires
.PHONY : CMakeFiles/TOUCH.dir/requires

CMakeFiles/TOUCH.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\TOUCH.dir\cmake_clean.cmake
.PHONY : CMakeFiles/TOUCH.dir/clean

CMakeFiles/TOUCH.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Project\Brain\SpatialIndex\cmake C:\Project\Brain\SpatialIndex\cmake C:\Project\Brain\SpatialIndex\build C:\Project\Brain\SpatialIndex\build C:\Project\Brain\SpatialIndex\build\CMakeFiles\TOUCH.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/TOUCH.dir/depend

