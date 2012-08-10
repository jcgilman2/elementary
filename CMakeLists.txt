
cmake_minimum_required (VERSION 2.6)

#### GCC OPTIMIZATION ####
IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of
build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug
Release RelWithDebInfo MinSizeRel.")
ELSE()
   SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build,
options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release
RelWithDebInfo MinSizeRel.")
ENDIF()
message(STATUS "Use build optimizations: " ${CMAKE_BUILD_TYPE})

#### Project Mia! ####

project (Mia)

#### Find dependencies ####

## Boost ##
find_package (Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries (bin/elementary ${Boost_PROGRAM_OPTIONS_LIBRARY})

#### Build Elementary ####
add_executable( bin/elementary src/elly/elly/main.cpp )
