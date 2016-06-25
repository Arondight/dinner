project (dinner C)
cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if(CMAKE_COMPILER_IS_GNUCC)
  set (CMAKE_C_FLAGS "-Wall")
  set (CMAKE_C_FLAGS_DEBUG "-g3")
  set (CMAKE_C_FLAGS_RELEASE "-O3")
  set (CMAKE_EXE_LINKER_FLAGS "-s")
endif()

if(NOT UNIX)
  message (FATAL_ERROR "Only Unix-like system is supported, quit.")
endif ()

include_directories (${PROJECT_SOURCE_DIR}/src)
add_subdirectory (${PROJECT_SOURCE_DIR}/src)

set (
  BINARYS
  ${PROJECT_SOURCE_DIR}/bin/dinner
)
install (PROGRAMS ${BINARYS} DESTINATION /usr/bin)

