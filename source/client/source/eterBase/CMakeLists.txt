cmake_minimum_required(VERSION 3.2)
project(EterBase)

add_definitions(-DNOMINMAX)

file(GLOB EterBase_HEADERS
    "*.h"
    "*.hpp"
    "Poly/*.h"
)
file(GLOB EterBase_SOURCES
    "*.c"
    "*.cpp"
    "Poly/*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${EterBase_HEADERS} ${EterBase_SOURCES})
