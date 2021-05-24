cmake_minimum_required(VERSION 3.2)
project(utilities)

add_subdirectory(${PROJECT_SOURCE_DIR}/FoxNet)
if(WIN32)
	if (CMAKE_SIZEOF_VOID_P EQUAL 4)
		add_subdirectory(${PROJECT_SOURCE_DIR}/VFSPro)
	endif()
endif()