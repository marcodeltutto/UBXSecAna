cmake_minimum_required (VERSION 3.0 FATAL_ERROR)
project (UBXSecAna)

# The version number.
set (UBXSecAna_VERSION_MAJOR 1)
set (UBXSecAna_VERSION_MINOR 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/Main/UBXSecAnaConfig.h.in"
  "${PROJECT_BINARY_DIR}/UBXSecAnaConfig.h"
)
install (FILES "${PROJECT_BINARY_DIR}/UBXSecAnaConfig.h" DESTINATION ${PROJECT_SOURCE_DIR}/Main)

#Set the C++ standard to c++11.
#set(CMAKE_CXX_STANDARD 11)

#Enable the use of runtime paths on Mac OSX.
#set(CMAKE_MACOSX_RPATH ON)

# You need to tell CMake where to find the ROOT installation.
# (https://root.cern.ch/how/integrate-root-my-project-cmake)
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS}/etc/cmake)
#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
find_package(ROOT REQUIRED COMPONENTS RIO)
if(!ROOT_FOUND)
    message(STATUS "PROBLEM FINDING ROOT, SEE BELOW")
    message(STATUS "ROOT found: ${ROOT_FOUND}")
    message(STATUS "ROOT_CXX_FLAGS: ${ROOT_CXX_FLAGS}")
    message(STATUS "ROOT_INCLUDE_DIRS: ${ROOT_INCLUDE_DIRS}")
    message(STATUS "ROOT_INCLUDE_DIRS: ${ROOT_INCLUDE_DIRS}")
    message(STATUS "ROOT_LIBRARIES: ${ROOT_LIBRARIES}")
    message(STATUS "ROOT_DEFINITIONS: ${ROOT_DEFINITIONS}")
    message(STATUS "ROOT_LIBRARIES: ${ROOT_LIBRARIES}")
    include(${ROOT_USE_FILE})
    include_directories(${ROOT_INCLUDE_DIRS})
endif()
include(${ROOT_USE_FILE})
#add_library(Event SHARED G__Event.cxx)
#target_link_libraries(Event ${ROOT_LIBRARIES})

#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
#include(${ROOT_USE_FILE})
include_directories(${ROOT_INCLUDE_DIRS})
link_directories(${ROOT_LIBRARY_DIR})

# add the binary tree to the search path for include files
# so that we will find UBXSecAnaConfig.h
include_directories("${PROJECT_BINARY_DIR}")

#set (CMAKE_CXX_COMPILER /usr/bin/g++)

add_definitions(`root-config --cflags --libs --glibs` -Wall -std=c++0x -std=c++1y  -std=gnu++1y)

subdirs(Main Libraries)

