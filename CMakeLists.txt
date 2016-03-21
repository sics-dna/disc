cmake_minimum_required (VERSION 2.6)
project (Isc2)

include_directories ("${PROJECT_SOURCE_DIR}")
set (EXTRA_LIBS ${EXTRA_LIBS} isc)

include_directories (../dataformat)

FILE (GLOB LIB_SRCS  "*.cc")
add_library(isc ${LIB_SRCS})
