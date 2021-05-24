cmake_minimum_required(VERSION 3.2)
project(EterImageLib)

file(GLOB EterImageLib_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB EterImageLib_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${EterImageLib_HEADERS} ${EterImageLib_SOURCES})
