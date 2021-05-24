cmake_minimum_required(VERSION 3.2)
project(MilesLib)

file(GLOB MilesLib_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB MilesLib_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${MilesLib_HEADERS} ${MilesLib_SOURCES})
