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
CMAKE_SOURCE_DIR = /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/uxncli.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/uxncli.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/uxncli.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/uxncli.dir/flags.make

CMakeFiles/uxncli.dir/src/uxncli.c.o: CMakeFiles/uxncli.dir/flags.make
CMakeFiles/uxncli.dir/src/uxncli.c.o: /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/uxncli.c
CMakeFiles/uxncli.dir/src/uxncli.c.o: CMakeFiles/uxncli.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/uxncli.dir/src/uxncli.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/uxncli.dir/src/uxncli.c.o -MF CMakeFiles/uxncli.dir/src/uxncli.c.o.d -o CMakeFiles/uxncli.dir/src/uxncli.c.o -c /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/uxncli.c

CMakeFiles/uxncli.dir/src/uxncli.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/uxncli.dir/src/uxncli.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/uxncli.c > CMakeFiles/uxncli.dir/src/uxncli.c.i

CMakeFiles/uxncli.dir/src/uxncli.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/uxncli.dir/src/uxncli.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/uxncli.c -o CMakeFiles/uxncli.dir/src/uxncli.c.s

CMakeFiles/uxncli.dir/src/uxn.c.o: CMakeFiles/uxncli.dir/flags.make
CMakeFiles/uxncli.dir/src/uxn.c.o: /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/uxn.c
CMakeFiles/uxncli.dir/src/uxn.c.o: CMakeFiles/uxncli.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/uxncli.dir/src/uxn.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/uxncli.dir/src/uxn.c.o -MF CMakeFiles/uxncli.dir/src/uxn.c.o.d -o CMakeFiles/uxncli.dir/src/uxn.c.o -c /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/uxn.c

CMakeFiles/uxncli.dir/src/uxn.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/uxncli.dir/src/uxn.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/uxn.c > CMakeFiles/uxncli.dir/src/uxn.c.i

CMakeFiles/uxncli.dir/src/uxn.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/uxncli.dir/src/uxn.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/uxn.c -o CMakeFiles/uxncli.dir/src/uxn.c.s

CMakeFiles/uxncli.dir/src/devices/system.c.o: CMakeFiles/uxncli.dir/flags.make
CMakeFiles/uxncli.dir/src/devices/system.c.o: /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/devices/system.c
CMakeFiles/uxncli.dir/src/devices/system.c.o: CMakeFiles/uxncli.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/uxncli.dir/src/devices/system.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/uxncli.dir/src/devices/system.c.o -MF CMakeFiles/uxncli.dir/src/devices/system.c.o.d -o CMakeFiles/uxncli.dir/src/devices/system.c.o -c /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/devices/system.c

CMakeFiles/uxncli.dir/src/devices/system.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/uxncli.dir/src/devices/system.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/devices/system.c > CMakeFiles/uxncli.dir/src/devices/system.c.i

CMakeFiles/uxncli.dir/src/devices/system.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/uxncli.dir/src/devices/system.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/devices/system.c -o CMakeFiles/uxncli.dir/src/devices/system.c.s

CMakeFiles/uxncli.dir/src/devices/file.c.o: CMakeFiles/uxncli.dir/flags.make
CMakeFiles/uxncli.dir/src/devices/file.c.o: /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/devices/file.c
CMakeFiles/uxncli.dir/src/devices/file.c.o: CMakeFiles/uxncli.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/uxncli.dir/src/devices/file.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/uxncli.dir/src/devices/file.c.o -MF CMakeFiles/uxncli.dir/src/devices/file.c.o.d -o CMakeFiles/uxncli.dir/src/devices/file.c.o -c /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/devices/file.c

CMakeFiles/uxncli.dir/src/devices/file.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/uxncli.dir/src/devices/file.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/devices/file.c > CMakeFiles/uxncli.dir/src/devices/file.c.i

CMakeFiles/uxncli.dir/src/devices/file.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/uxncli.dir/src/devices/file.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/devices/file.c -o CMakeFiles/uxncli.dir/src/devices/file.c.s

CMakeFiles/uxncli.dir/src/devices/datetime.c.o: CMakeFiles/uxncli.dir/flags.make
CMakeFiles/uxncli.dir/src/devices/datetime.c.o: /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/devices/datetime.c
CMakeFiles/uxncli.dir/src/devices/datetime.c.o: CMakeFiles/uxncli.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/uxncli.dir/src/devices/datetime.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/uxncli.dir/src/devices/datetime.c.o -MF CMakeFiles/uxncli.dir/src/devices/datetime.c.o.d -o CMakeFiles/uxncli.dir/src/devices/datetime.c.o -c /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/devices/datetime.c

CMakeFiles/uxncli.dir/src/devices/datetime.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/uxncli.dir/src/devices/datetime.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/devices/datetime.c > CMakeFiles/uxncli.dir/src/devices/datetime.c.i

CMakeFiles/uxncli.dir/src/devices/datetime.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/uxncli.dir/src/devices/datetime.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/src/devices/datetime.c -o CMakeFiles/uxncli.dir/src/devices/datetime.c.s

# Object files for target uxncli
uxncli_OBJECTS = \
"CMakeFiles/uxncli.dir/src/uxncli.c.o" \
"CMakeFiles/uxncli.dir/src/uxn.c.o" \
"CMakeFiles/uxncli.dir/src/devices/system.c.o" \
"CMakeFiles/uxncli.dir/src/devices/file.c.o" \
"CMakeFiles/uxncli.dir/src/devices/datetime.c.o"

# External object files for target uxncli
uxncli_EXTERNAL_OBJECTS =

uxncli: CMakeFiles/uxncli.dir/src/uxncli.c.o
uxncli: CMakeFiles/uxncli.dir/src/uxn.c.o
uxncli: CMakeFiles/uxncli.dir/src/devices/system.c.o
uxncli: CMakeFiles/uxncli.dir/src/devices/file.c.o
uxncli: CMakeFiles/uxncli.dir/src/devices/datetime.c.o
uxncli: CMakeFiles/uxncli.dir/build.make
uxncli: CMakeFiles/uxncli.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking C executable uxncli"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/uxncli.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/uxncli.dir/build: uxncli
.PHONY : CMakeFiles/uxncli.dir/build

CMakeFiles/uxncli.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/uxncli.dir/cmake_clean.cmake
.PHONY : CMakeFiles/uxncli.dir/clean

CMakeFiles/uxncli.dir/depend:
	cd /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/cmake-build-debug /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/cmake-build-debug /Users/baohongbin/Desktop/UxnVM-SYCL/UxnSourceCode/uxn/cmake-build-debug/CMakeFiles/uxncli.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/uxncli.dir/depend

