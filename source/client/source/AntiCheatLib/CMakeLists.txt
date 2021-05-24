cmake_minimum_required(VERSION 3.2)
project(AntiCheatLib)

file(GLOB AntiCheatLib_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB AntiCheatLib_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${AntiCheatLib_HEADERS} ${AntiCheatLib_SOURCES})
