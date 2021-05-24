cmake_minimum_required(VERSION 3.2)
project(SpeedTreeLib)

file(GLOB SpeedTreeLib_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB SpeedTreeLib_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${SpeedTreeLib_HEADERS} ${SpeedTreeLib_SOURCES})
