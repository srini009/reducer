# (C) 2020 The University of Chicago 
# See COPYRIGHT in top-level directory.
cmake_minimum_required (VERSION 3.8)
project (reducer C CXX) # add CXX if needed
enable_testing ()

add_definitions (-g -Wextra -Wall -Wpedantic -fpermissive)

option (ENABLE_TESTS    "Build tests" OFF)
option (ENABLE_AGGREGATOR   "Build the aggregator module" ON)
option (ENABLE_BEDROCK  "Build bedrock module" OFF)
option (ENABLE_SYMBIOMON  "Build symbiomon module" OFF)

# add our cmake module directory to the path
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# link shared lib with full rpath
set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# setup cache variables for ccmake
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release
         CACHE STRING "Choose the type of build." FORCE)
    set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                  "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif ()
set (CMAKE_PREFIX_PATH "" CACHE STRING "External dependencies path")
set (BUILD_SHARED_LIBS "ON" CACHE BOOL "Build a shared library")

find_package (PkgConfig REQUIRED)

if (${ENABLE_BEDROCK})
    find_package(bedrock REQUIRED)
endif ()

if (${ENABLE_SYMBIOMON})
    find_package(symbiomon REQUIRED)
endif ()

if (${ENABLE_AGGREGATOR})
    pkg_check_modules (SDSKV REQUIRED IMPORTED_TARGET sdskv-client)
endif ()

# search for margo
pkg_check_modules (MARGO REQUIRED IMPORTED_TARGET margo)
# search for uuid
pkg_check_modules (UUID REQUIRED IMPORTED_TARGET uuid)
# comment if you don't need abt-io
pkg_check_modules (ABTIO REQUIRED IMPORTED_TARGET abt-io)
# search for json-c
#pkg_check_modules (JSONC REQUIRED IMPORTED_TARGET json-c)

# library version set here (e.g. for shared libs).
set (REDUCER_VERSION_MAJOR 0)
set (REDUCER_VERSION_MINOR 1)
set (REDUCER_VERSION_PATCH 0)
set (REDUCER_VERSION
     "${REDUCER_VERSION_MAJOR}.${REDUCER_VERSION_MINOR}.${REDUCER_VERSION_PATCH}")

add_subdirectory (src)
if(${ENABLE_TESTS})
  add_subdirectory (tests)
endif(${ENABLE_TESTS})

if(${ENABLE_SYMBIOMON})
  add_subdirectory (bin)
endif(${ENABLE_SYMBIOMON})
