cmake_minimum_required(VERSION 3.2)
project(gamelib CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./lib)

include_directories("./include")
include_directories("../../../extern/include")

file(GLOB GAMELIB_HEADERS
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
)
file(GLOB GAMELIB_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.cc"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${GAMELIB_HEADERS} ${GAMELIB_SOURCES})
