cmake_minimum_required(VERSION 3.2)
project(EffectLib)

file(GLOB EffectLib_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB EffectLib_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${EffectLib_HEADERS} ${EffectLib_SOURCES})
