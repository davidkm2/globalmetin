cmake_minimum_required(VERSION 3.2)
project(EterLib)

file(GLOB EterLib_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB EterLib_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${EterLib_HEADERS} ${EterLib_SOURCES})
