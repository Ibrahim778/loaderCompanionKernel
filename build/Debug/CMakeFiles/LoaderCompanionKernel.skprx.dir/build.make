# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/ibrahim/GitRepos/KernelLaunchAppExport

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ibrahim/GitRepos/KernelLaunchAppExport/build/Debug

# Utility rule file for LoaderCompanionKernel.skprx.

# Include the progress variables for this target.
include CMakeFiles/LoaderCompanionKernel.skprx.dir/progress.make

CMakeFiles/LoaderCompanionKernel.skprx: LoaderCompanionKernel.skprx.out
	/usr/bin/cmake -E copy /home/ibrahim/GitRepos/KernelLaunchAppExport/build/Debug/LoaderCompanionKernel.skprx.out LoaderCompanionKernel.skprx

LoaderCompanionKernel.skprx.out: LoaderCompanionKernel.velf
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/ibrahim/GitRepos/KernelLaunchAppExport/build/Debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Creating SELF LoaderCompanionKernel.skprx"
	/usr/local/vitasdk/bin/vita-make-fself -c /home/ibrahim/GitRepos/KernelLaunchAppExport/build/Debug/LoaderCompanionKernel.velf /home/ibrahim/GitRepos/KernelLaunchAppExport/build/Debug/LoaderCompanionKernel.skprx.out

LoaderCompanionKernel.velf: LoaderCompanionKernel
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/ibrahim/GitRepos/KernelLaunchAppExport/build/Debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Converting to Sony ELF LoaderCompanionKernel.velf"
	/usr/local/vitasdk/bin/vita-elf-create -e /home/ibrahim/GitRepos/KernelLaunchAppExport/LoaderCompanionKernel.yml /home/ibrahim/GitRepos/KernelLaunchAppExport/build/Debug/LoaderCompanionKernel /home/ibrahim/GitRepos/KernelLaunchAppExport/build/Debug/LoaderCompanionKernel.velf

LoaderCompanionKernel.skprx: CMakeFiles/LoaderCompanionKernel.skprx
LoaderCompanionKernel.skprx: LoaderCompanionKernel.skprx.out
LoaderCompanionKernel.skprx: LoaderCompanionKernel.velf
LoaderCompanionKernel.skprx: CMakeFiles/LoaderCompanionKernel.skprx.dir/build.make

.PHONY : LoaderCompanionKernel.skprx

# Rule to build all files generated by this target.
CMakeFiles/LoaderCompanionKernel.skprx.dir/build: LoaderCompanionKernel.skprx

.PHONY : CMakeFiles/LoaderCompanionKernel.skprx.dir/build

CMakeFiles/LoaderCompanionKernel.skprx.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/LoaderCompanionKernel.skprx.dir/cmake_clean.cmake
.PHONY : CMakeFiles/LoaderCompanionKernel.skprx.dir/clean

CMakeFiles/LoaderCompanionKernel.skprx.dir/depend:
	cd /home/ibrahim/GitRepos/KernelLaunchAppExport/build/Debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ibrahim/GitRepos/KernelLaunchAppExport /home/ibrahim/GitRepos/KernelLaunchAppExport /home/ibrahim/GitRepos/KernelLaunchAppExport/build/Debug /home/ibrahim/GitRepos/KernelLaunchAppExport/build/Debug /home/ibrahim/GitRepos/KernelLaunchAppExport/build/Debug/CMakeFiles/LoaderCompanionKernel.skprx.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/LoaderCompanionKernel.skprx.dir/depend
