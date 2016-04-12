cmake_minimum_required(VERSION 2.4)

###
# Compiler flags
#

# GCC
if (CMAKE_COMPILER_IS_GNUCC)
  set (CMAKE_C_FLAGS "-O2 -Wall")
  set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
endif (CMAKE_COMPILER_IS_GNUCC)

###
# Project Stuff
#

# Base include directory
include_directories(include)

# Begin build
subdirs(src)