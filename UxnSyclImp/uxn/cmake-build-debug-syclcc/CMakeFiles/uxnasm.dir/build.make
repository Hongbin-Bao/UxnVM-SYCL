# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

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
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn/cmake-build-debug-syclcc

# Include any dependencies generated for this target.
include CMakeFiles/uxnasm.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/uxnasm.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/uxnasm.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/uxnasm.dir/flags.make

CMakeFiles/uxnasm.dir/src/uxnasm.c.o: CMakeFiles/uxnasm.dir/flags.make
CMakeFiles/uxnasm.dir/src/uxnasm.c.o: /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn/src/uxnasm.c
CMakeFiles/uxnasm.dir/src/uxnasm.c.o: CMakeFiles/uxnasm.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn/cmake-build-debug-syclcc/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/uxnasm.dir/src/uxnasm.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/uxnasm.dir/src/uxnasm.c.o -MF CMakeFiles/uxnasm.dir/src/uxnasm.c.o.d -o CMakeFiles/uxnasm.dir/src/uxnasm.c.o -c /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn/src/uxnasm.c

CMakeFiles/uxnasm.dir/src/uxnasm.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/uxnasm.dir/src/uxnasm.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn/src/uxnasm.c > CMakeFiles/uxnasm.dir/src/uxnasm.c.i

CMakeFiles/uxnasm.dir/src/uxnasm.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/uxnasm.dir/src/uxnasm.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn/src/uxnasm.c -o CMakeFiles/uxnasm.dir/src/uxnasm.c.s

# Object files for target uxnasm
uxnasm_OBJECTS = \
"CMakeFiles/uxnasm.dir/src/uxnasm.c.o"

# External object files for target uxnasm
uxnasm_EXTERNAL_OBJECTS =

uxnasm: CMakeFiles/uxnasm.dir/src/uxnasm.c.o
uxnasm: CMakeFiles/uxnasm.dir/build.make
uxnasm: CMakeFiles/uxnasm.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn/cmake-build-debug-syclcc/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable uxnasm"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/uxnasm.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/uxnasm.dir/build: uxnasm
.PHONY : CMakeFiles/uxnasm.dir/build

CMakeFiles/uxnasm.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/uxnasm.dir/cmake_clean.cmake
.PHONY : CMakeFiles/uxnasm.dir/clean

CMakeFiles/uxnasm.dir/depend:
	cd /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn/cmake-build-debug-syclcc && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn/cmake-build-debug-syclcc /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn/cmake-build-debug-syclcc /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSyclImp/uxn/cmake-build-debug-syclcc/CMakeFiles/uxnasm.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/uxnasm.dir/depend
