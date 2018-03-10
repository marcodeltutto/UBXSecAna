cmake_minimum_required (VERSION 2.6)
project (UBXSecAna)

# The version number.
set (UBXSecAna_VERSION_MAJOR 1)
set (UBXSecAna_VERSION_MINOR 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/Main/Include/UBXSecAnaConfig.h.in"
  "${PROJECT_BINARY_DIR}/UBXSecAnaConfig.h"
)

# You need to tell CMake where to find the ROOT installation.
# (https://root.cern.ch/how/integrate-root-my-project-cmake)
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
#include(${ROOT_USE_FILE})

# add the binary tree to the search path for include files
# so that we will find UBXSecAnaConfig.h
include_directories("${PROJECT_BINARY_DIR}")

#set (CMAKE_CXX_COMPILER /usr/bin/g++)

add_definitions(`root-config --cflags --libs --glibs` -Wall -std=c++0x -Qunused-arguments)

subdirs(Main Libraries)


# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
#include_directories("${PROJECT_BINARY_DIR}")

# add the executable
#add_executable(Maker Maker.cpp)

