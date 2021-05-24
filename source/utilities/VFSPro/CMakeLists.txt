cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(VFSPro)

# Output folder
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../../Bin/Utilities)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# C++ Standard 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# General linker options
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /NODEFAULTLIB:libci.lib")

# Sub projects
add_subdirectory(${PROJECT_SOURCE_DIR}/VFSCryptLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/VFSLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/VFSArchiver)
add_subdirectory(${PROJECT_SOURCE_DIR}/VFSTest)
add_subdirectory(${PROJECT_SOURCE_DIR}/VFSPropertyGenerator)
