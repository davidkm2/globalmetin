cmake_minimum_required(VERSION 3.2)
project(UserInterface)

set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../../../Bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR})

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(EXE_NAME client_debug_${TARGET_ARCH_STR})
else()
	set(EXE_NAME client_release_${TARGET_ARCH_STR})
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	add_definitions(-D_DEBUG)
	add_definitions(-DDEBUG)
	add_definitions(-DUSE_LOD)
	add_definitions(-DDUNGEON_WORK)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
	add_definitions(-DDISTRIBUTE_CYTHON=1)
	add_definitions(-D_DISTRIBUTE)
	add_definitions(-DUSE_LOD)
	add_definitions(-DDUNGEON_WORK)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
	add_definitions(-DDISTRIBUTE_CYTHON=1)
	add_definitions(-D_DISTRIBUTE)
	add_definitions(-DUSE_LOD)
	add_definitions(-DDUNGEON_WORK)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
	add_definitions(-DDISTRIBUTE_CYTHON=1)
	add_definitions(-D_DISTRIBUTE)
	add_definitions(-DUSE_LOD)
	add_definitions(-DDUNGEON_WORK)
endif()

file(GLOB UserInterface_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB UserInterface_SOURCES
    "*.c"
    "*.cpp"
)
file(GLOB UserInterface_OTHER_INPUT
    "*.rc"
)
add_executable(${EXE_NAME} WIN32 ${UserInterface_HEADERS} ${UserInterface_SOURCES} ${UserInterface_OTHER_INPUT})

SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES LINK_FLAGS    "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS")
target_link_libraries(${EXE_NAME} EterBase EffectLib GameLib EterGrnLib EterImageLib EterLib eterPack EterPythonLib ScriptLib AntiCheatLib MilesLib SphereLib PRTerrainLib SpeedTreeLib CWebBrowser ${EXTRA_LIBS})

ADD_CUSTOM_COMMAND(TARGET ${EXE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${EXE_NAME}.exe ${PROJECT_BINARY_DIR}/../../../../../../Client/Titan2/${EXE_NAME}_new.exe
)
