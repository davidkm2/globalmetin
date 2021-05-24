cmake_minimum_required(VERSION 3.8)
project(VFSPropertyGenerator)
set(EXE_NAME VFSPropertyGenerator)

# Preprocessor flags
if (WIN32)
	add_definitions(-DWIN32)
	add_definitions(-D_WIN32)
endif()

add_definitions(-DNOMINMAX)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	add_definitions(-D_DEBUG)
    add_definitions(-DDEBUG)
    set(BUILD_MODE "debug")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
    set(BUILD_MODE "normal")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
    set(BUILD_MODE "normal")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
    set(BUILD_MODE "normal")
endif()

# Project files
set(PROPERTYMGR_HEADERS
)
set(PROPERTYMGR_SOURCES
	${PROJECT_SOURCE_DIR}/src/main.cpp
)

set(EXTRA_LIBS dbghelp VFSCryptLib VFSLib)
add_executable(${EXE_NAME}
    ${PROPERTYMGR_HEADERS}
    ${PROPERTYMGR_SOURCES}
)

target_link_libraries(${EXE_NAME} ${EXTRA_LIBS})

add_custom_command(
    TARGET ${EXE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "LINKING: ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${EXE_NAME}.exe ${PROJECT_BINARY_DIR}/../../../../../../Client/Titan2/CRYPTION/source/${EXE_NAME}.exe"
    COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${EXE_NAME}.exe ${PROJECT_BINARY_DIR}/../../../../../../Client/Titan2/CRYPTION/source/${EXE_NAME}.exe
)