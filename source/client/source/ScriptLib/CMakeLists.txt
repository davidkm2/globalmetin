cmake_minimum_required(VERSION 3.2)
project(ScriptLib)

file(GLOB ScriptLib_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB ScriptLib_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${ScriptLib_HEADERS} ${ScriptLib_SOURCES})
