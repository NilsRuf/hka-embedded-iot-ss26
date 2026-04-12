# This file is used by the build system to find sources you want to compile.

# Add our DTS folders, so Zephyr can discover them.
list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Some basic setup stuff. We won't go into great detail.
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

# This adds our C files to the project.
target_sources(app PRIVATE
    src/main.c
    # Other source files go here
    )

# Add everything under include/ to the include path.
zephyr_include_directories(include)

# Add sources for drivers as well.
add_subdirectory(drivers)

# Enable more warnings
target_compile_options(app PRIVATE -Wall -Wextra)
