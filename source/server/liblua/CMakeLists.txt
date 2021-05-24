cmake_minimum_required(VERSION 3.2)
project(lualib C)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./lib)

include_directories("./include")
include_directories("../../../extern/include")	

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

file(GLOB LUALIB_HEADERS
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
)
file(GLOB LUALIB_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${LUALIB_HEADERS} ${LUALIB_SOURCES})
