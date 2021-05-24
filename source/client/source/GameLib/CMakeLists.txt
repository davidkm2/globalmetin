cmake_minimum_required(VERSION 3.2)
project(GameLib)

file(GLOB GameLib_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB GameLib_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${GameLib_HEADERS} ${GameLib_SOURCES})
