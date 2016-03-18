cmake_minimum_required (VERSION 2.6)
project (dataformat)

include_directories ("${PROJECT_SOURCE_DIR}")
set (EXTRA_LIBS ${EXTRA_LIBS} dataformat)

add_library(dataformat readtokens.cc table.cc format.cc formatdispatch.cc formatbinary.cc formatdiscr.cc formatcont.cc formatsymbol.cc formatunknown.cc data.cc datadispatch.cc)

