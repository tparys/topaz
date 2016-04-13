cmake_minimum_required(VERSION 2.4)

###
# Doxygen target
#

find_package (Doxygen)
if (DOXYGEN_FOUND)
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target (doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif (DOXYGEN_FOUND)

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
