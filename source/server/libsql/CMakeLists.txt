cmake_minimum_required(VERSION 3.2)
project(sqllib CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./lib)

include_directories("../../../extern/include")	
include_directories("./include/")

file(GLOB SQLLIB_HEADERS
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
)
file(GLOB SQLLIB_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${SQLLIB_HEADERS} ${SQLLIB_SOURCES})
