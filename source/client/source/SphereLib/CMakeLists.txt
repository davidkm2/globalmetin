cmake_minimum_required(VERSION 3.2)
project(SphereLib)

file(GLOB SphereLib_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB SphereLib_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${SphereLib_HEADERS} ${SphereLib_SOURCES})
