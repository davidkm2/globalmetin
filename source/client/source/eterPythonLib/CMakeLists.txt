cmake_minimum_required(VERSION 3.2)
project(EterPythonLib)

file(GLOB EterPythonLib_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB EterPythonLib_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${EterPythonLib_HEADERS} ${EterPythonLib_SOURCES})
