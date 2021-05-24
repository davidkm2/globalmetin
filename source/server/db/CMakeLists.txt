cmake_minimum_required(VERSION 3.2)
project(db_cache CXX)
set(EXE_NAME db_cache)

# Output dir
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../../Bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR})

# Preprocessor flags
if (MSVC)
	add_definitions(-DWIN32)
	add_definitions(-D_WIN32)
endif()

#add_definitions(-D_USE_32BIT_TIME_T)
add_definitions(-DNOMINMAX)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	add_definitions(-D_DEBUG)
	add_definitions(-DDEBUG)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
	add_definitions(-D_NDEBUG)
	add_definitions(-DNDEBUG)
endif()

# Project files

file(GLOB DBCACHE_HEADERS
    "src/*.h"
    "src/*.hpp"
)
file(GLOB DBCACHE_SOURCES
    "src/*.c"
    "src/*.cpp"
)

if (MSVC)
	set(EXTRA_LIBS ws2_32.lib dbghelp.lib)

	if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		set(EXTRA_LIBS ${EXTRA_LIBS} mysql\\x${TARGET_ARCH_STR}\\debug\\mysqlclient)
	else()
		set(EXTRA_LIBS ${EXTRA_LIBS} mysql\\x${TARGET_ARCH_STR}\\release\\mysqlclient)
	endif()

	set(EXTRA_LIBS ${EXTRA_LIBS} DevIL_x${TARGET_ARCH_STR})

	add_executable(${EXE_NAME} ${DBCACHE_HEADERS} ${DBCACHE_SOURCES})
	
	target_link_libraries(${EXE_NAME} thecorelib sqllib gamelib ${EXTRA_LIBS})
#	set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCPMT")

else()
	add_executable(${EXE_NAME} ${DBCACHE_HEADERS} ${DBCACHE_SOURCES})

	FIND_LIBRARY(MYSQL_LIBRARY libmysqlclient.a ${MYSQL_DIR}/lib NO_DEFAULT_PATH)
#	FIND_LIBRARY(LIBZ libz.a /usr/lib)

	# Static
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

	# Link
	target_link_libraries(${EXE_NAME} ${LIBZ} ${MYSQL_LIBRARY} pthread -static-libgcc -static-libstdc++ thecorelib sqllib gamelib)

endif()

SET_TARGET_PROPERTIES(${exe} PROPERTIES COMPILE_DEFINITIONS "${_definitions}") 
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	SET_TARGET_PROPERTIES(${exe} PROPERTIES LINK_SEARCH_END_STATIC 1) #STATIC LINKING for clang
	set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic 
	set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS) 
	set(CMAKE_SHARED_LIBRARY_C_FLAGS)         # remove -fPIC 
	set(CMAKE_SHARED_LIBRARY_CXX_FLAGS) 
	set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)    # remove -rdynamic 
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS) 
endif()

if (WIN32)
    ADD_CUSTOM_COMMAND(
        TARGET ${EXE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${EXE_NAME}.exe ${PROJECT_BINARY_DIR}/../../../../../Server/share/bin/${EXE_NAME}.exe
    )
endif()