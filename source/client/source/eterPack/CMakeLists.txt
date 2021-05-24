cmake_minimum_required(VERSION 3.2)
project(eterPack)

file(GLOB ETERPACK_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ETERPACK_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ETERPACK_HEADERS} ${ETERPACK_SOURCES})
