cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(Titan2)

message(STATUS "CMake version: ${CMAKE_VERSION}")

# Default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if (MSVC)
	message(STATUS "MSVC: ${MSVC_VERSION}")
endif()

# LGBT mode on
SET(CMAKE_COLOR_MAKEFILE ON)

# Settings
option(BUILD_UTILITIES "Build the common utilities" OFF)
option(BUILD_SERVER "Build the server binaries" ON)
option(BUILD_CLIENT "Build the client binaries" ON)
option(BUILD_TOOLS "Build the tool binaries" OFF)

# Git
execute_process ( COMMAND git rev-list HEAD OUTPUT_VARIABLE GIT_REV_LIST_OUTPUT ERROR_VARIABLE GIT_REV_LIST_OUTPUT RESULT_VARIABLE GIT_REV_LIST_RESULT WORKING_DIRECTORY "." )

if( NOT ${GIT_REV_LIST_RESULT} EQUAL 0 )
	message( FATAL_ERROR "get_git_revision(): can not get git revision. log followed: ${GIT_REV_LIST_OUTPUT}")
endif()

string(STRIP ${GIT_REV_LIST_OUTPUT} GIT_REV_LIST_OUTPUT)
string(REPLACE "\n" ";" GIT_REV_LIST ${GIT_REV_LIST_OUTPUT} )

list(LENGTH GIT_REV_LIST gitVersion )
MESSAGE(STATUS "GitREV=${gitVersion}")

add_definitions(-D_GIT_VERSION_="${gitVersion}")
if (WIN32)
    add_definitions(-D_HOSTNAME_="${USERNAME}")
else()
    add_definitions(-D_HOSTNAME_="${HOSTNAME}")
endif()
add_definitions(-D_DIRECTORY_="${CMAKE_SOURCE_DIR}")

# Until to find client x64 3rd libs
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	# Disable client build for x64
	set(BUILD_CLIENT OFF)
endif()

# Platform defination
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	add_definitions(-DTARGET_ARCH=64)
	set(TARGET_ARCH_STR "64")
else() 
	add_definitions(-DTARGET_ARCH=32)
	set(TARGET_ARCH_STR "86")
endif()

# Compiler subdir for MySQL libs
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(COMPILER_SUBDIR "clang")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(COMPILER_SUBDIR "gcc")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	set(COMPILER_SUBDIR "intel")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(COMPILER_SUBDIR "msvc")
endif()

if(NOT WIN32)
	# Disable client build for non win32
	set(BUILD_CLIENT OFF)
	
	# Register MySQL path
	set(MYSQL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/freebsd_libs_x${TARGET_ARCH_STR}/mysql/${COMPILER_SUBDIR})
endif()

# Check enabled projects
if("${BUILD_SERVER}" STREQUAL "OFF" AND "${BUILD_CLIENT}" STREQUAL "OFF" AND "${BUILD_TOOLS}" STREQUAL "OFF")
	message(FATAL_ERROR "Nothing to build")
	return()
endif()

# CMake modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/extern/cmake")
add_definitions(-DCMAKE_BUILD_SYSTEM)

# C++ Standard 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC specific settings ( MD -> MT, Multi processor, Disable SAFESH, Disable linking libci.lib )
if(MSVC)
    set(LIB_RT_SUFFIX "mt")
    set(LIB_RT_OPTION "/MT")

    foreach(flag_var  CMAKE_C_FLAGS  CMAKE_CXX_FLAGS)
         foreach(config_name  ""  DEBUG  RELEASE  MINSIZEREL  RELWITHDEBINFO)
            set(var_name "${flag_var}")

            if(NOT "${config_name}" STREQUAL "")
                set(var_name "${var_name}_${config_name}")
            endif()
            
            string(REPLACE "/MD" "${LIB_RT_OPTION}" ${var_name} "${${var_name}}")
            set(${var_name} "${${var_name}}" CACHE STRING "" FORCE)
        endforeach()
    endforeach()

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /NODEFAULTLIB:libci.lib")
endif()

# Register 3rd dependecies include paths
include_directories(${PROJECT_SOURCE_DIR}/extern/include)
include_directories(${PROJECT_SOURCE_DIR}/extern/include/d3d9)
include_directories(${PROJECT_SOURCE_DIR}/extern/3rd_party)
include_directories(${PROJECT_SOURCE_DIR}/extern/3rd_party/lzo/include)
include_directories(${PROJECT_SOURCE_DIR}/extern/3rd_party/mysql-connector-c-6.1.11-src/include)
#include_directories(${PROJECT_SOURCE_DIR}/extern/3rd_party/mariadb-connector-c-2.2.3-src/src/include)
include_directories(${PROJECT_SOURCE_DIR}/extern/3rd_party/DevIL/DevIL/include)
include_directories(${PROJECT_SOURCE_DIR}/extern/3rd_party/libuv/include)
include_directories(${PROJECT_SOURCE_DIR}/extern/3rd_party/uvw/src)
include_directories(${PROJECT_SOURCE_DIR}/extern/3rd_party/spdlog/include)
include_directories(${PROJECT_SOURCE_DIR}/extern/3rd_party/lz4/lib)
include_directories(${PROJECT_SOURCE_DIR}/extern/3rd_party/tbb/include)
include_directories(${PROJECT_SOURCE_DIR}/extern/3rd_party/asio/asio/include)

# Register 3rd dependecies precompiled library path
link_directories(${PROJECT_SOURCE_DIR}/Bin/Utilities)
link_directories(${PROJECT_SOURCE_DIR}/extern/lib_2019)
if (NOT MSVC)
	link_directories(${PROJECT_SOURCE_DIR}/extern/freebsd_libs_x${TARGET_ARCH_STR})
endif()

# Sub-projects
if(BUILD_UTILITIES)
	add_subdirectory(${PROJECT_SOURCE_DIR}/source/utilities)
endif()

if(BUILD_SERVER)
	add_subdirectory(${PROJECT_SOURCE_DIR}/source/server)
endif()

if(WIN32)
	if(BUILD_CLIENT)
		add_subdirectory(${PROJECT_SOURCE_DIR}/source/client)
	endif()
	if(BUILD_TOOLS)
		add_subdirectory(${PROJECT_SOURCE_DIR}/tools)
	endif()
endif()
