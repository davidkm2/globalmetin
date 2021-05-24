cmake_minimum_required(VERSION 3.2)
project(PRTerrainLib)

file(GLOB PRTerrainLib_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB PRTerrainLib_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${PRTerrainLib_HEADERS} ${PRTerrainLib_SOURCES})
