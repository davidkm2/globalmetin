cmake_minimum_required(VERSION 3.2)
project(client)

add_definitions(-DNOMINMAX)
add_definitions(-DBUILDING_GRANNY_STATIC=1)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	add_definitions(-D_ITERATOR_DEBUG_LEVEL=0) # HACK: Required for speedtree linking on debug build, It's also requires the same change to make sub dependencies such as cryptopp and tinyxml2.
endif()

# rpcndr.h(192,14): error C2872:  'byte': ambiguous symbol
# cstddef(24,12): message :  or       'std::byte' 
add_definitions(-D_HAS_STD_BYTE=0)

# Windows headers require the default packing option. Changing this can lead to memory corruption
add_definitions(-DWINDOWS_IGNORE_PACKING_MISMATCH)

set(WARNINGS_DISABLE
	# ignore
	5033 #  (cpython/2.7.16) 'register' is no longer a supported storage class 
)

foreach(d ${WARNINGS_DISABLE})
	set(WARNINGS "${WARNINGS} /wd${d}")
endforeach(d)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS} ${MT_BUILD}")


include_directories(${PROJECT_SOURCE_DIR}/../utilities/VFSPro/VFSLib/include)

add_subdirectory(${PROJECT_SOURCE_DIR}/source/EterBase)
## add_subdirectory(${PROJECT_SOURCE_DIR}/source/CythonLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/EffectLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/GameLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/EterGrnLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/EterImageLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/EterLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/EterPack)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/EterPythonLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/ScriptLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/AntiCheatLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/MilesLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/SphereLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/PRTerrainLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/SpeedTreeLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/CWebBrowser)
add_subdirectory(${PROJECT_SOURCE_DIR}/source/UserInterface)
