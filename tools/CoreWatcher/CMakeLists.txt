cmake_minimum_required(VERSION 3.8)
project(CoreWatcher)

set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../Bin/Tools)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

add_executable (${PROJECT_NAME}
    src/main.cpp src/FileMonitor.cpp
)

if(NOT MSVC)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} stdc++ stdc++fs)
endif()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-DPLATFORM=64)
else (CMAKE_SIZEOF_VOID_P EQUAL 8) 
    add_definitions(-DPLATFORM=32)
endif ()
