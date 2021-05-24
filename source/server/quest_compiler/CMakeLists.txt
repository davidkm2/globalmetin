cmake_minimum_required(VERSION 3.2)
project(qc)
set(EXE_NAME qc)

# Output dir
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../../Bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR})

set(QC_SOURCES
	${PROJECT_SOURCE_DIR}/src/qc.cc
	${PROJECT_SOURCE_DIR}/src/crc32.cc
)
set(QC_HEADERS
	${PROJECT_SOURCE_DIR}/include/crc32.h		
)

include_directories("../liblua/include")
include_directories("../liblua/src")

link_directories(${PROJECT_SOURCE_DIR}/../liblua/lib)

add_executable(${EXE_NAME} ${QC_SOURCES} ${QC_HEADERS})
target_link_libraries(${EXE_NAME} lualib)

if (WIN32)
    add_custom_command(
        TARGET ${EXE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "LINKING: ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${EXE_NAME}.exe ${PROJECT_BINARY_DIR}/../../../../../Server/share/locale/turkey/quest/${EXE_NAME}.exe"
        COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${EXE_NAME}.exe ${PROJECT_BINARY_DIR}/../../../../../Server/share/locale/turkey/quest/${EXE_NAME}.exe
    )
endif()