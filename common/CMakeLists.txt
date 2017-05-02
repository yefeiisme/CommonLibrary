cmake_minimum_required (VERSION 2.8)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O2 -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -g")
SET(CMAKE_C_COMPILER g++)

add_compile_options(-std=c++11)

project (common)
if (NOT SET_OUTPUT_DIRECTORY)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/target")
    set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "../../Share/llib")
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "../../Share/llib")
    set (SET_OUTPUT_DIRECTORY ON)
endif (NOT SET_OUTPUT_DIRECTORY)

add_subdirectory ("Common")
add_subdirectory ("Daemon")
add_subdirectory ("FileLog")
add_subdirectory ("FileReader")
add_subdirectory ("Network")
add_subdirectory ("RingBuffer")
