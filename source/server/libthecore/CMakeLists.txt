cmake_minimum_required(VERSION 3.2)
project(thecorelib CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./lib)

include_directories("../../../extern/include")	
include_directories("./include/")

file(GLOB THECORELIB_HEADERS
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
)
file(GLOB THECORELIB_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_definitions(-DNOMINMAX)

add_library(${PROJECT_NAME} STATIC ${THECORELIB_HEADERS} ${THECORELIB_SOURCES})
