cmake_minimum_required(VERSION 3.8)
project(DumpProto)

set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../Bin/Tools)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

file(GLOB HEADER_FILES
    "include/*.h"
    "include/*.hpp"
)
file(GLOB SOURCE_FILES
    "src/*.c"
    "src/*.cpp"
)
file(GLOB OTHER_INPUT_FILES
    "*.rc"
)

add_executable (${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${OTHER_INPUT_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)

