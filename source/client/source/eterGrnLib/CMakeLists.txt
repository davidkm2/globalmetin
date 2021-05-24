cmake_minimum_required(VERSION 3.2)
project(EterGrnLib)

file(GLOB EterGrnLib_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB EterGrnLib_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${EterGrnLib_HEADERS} ${EterGrnLib_SOURCES})
