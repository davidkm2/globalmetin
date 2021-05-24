cmake_minimum_required(VERSION 3.2)
project(server)

set(OUTPUT_DIR  ${PROJECT_SOURCE_DIR}/../../Bin)

# Setting global warning settings
if(MSVC)
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
	endif()

	set(WARNINGS_DISABLE
		# ignore
		4100 # 'identifier' : unreferenced formal parameter
		4996 # function or variable may be unsafe(_s)
		4706 # assignment within conditional expression
		4244 # 'argument': conversion from 'const wchar_t' to 'const _Elem', possible loss of data
	)

	foreach(d ${WARNINGS_DISABLE})
		set(WARNINGS "${WARNINGS} /wd${d}")
	endforeach(d)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS} ${MT_BUILD}")

elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

# Non-MSVC specific
if(NOT MSVC)
#	# x86 build
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
#	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

	# Pipe
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe")

	# Disable encoding warnings
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-source-encoding")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-invalid-source-encoding")
	endif()

	# TODO: Try ASAN
	# -fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer

	# Threading
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -D_THREAD_SAFE")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -D_THREAD_SAFE")

	# Debug flags
	if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fexceptions")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fexceptions")
	endif()

	# Stack protector
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")

endif()

# Alt projects
add_subdirectory(libthecore)
add_subdirectory(libsql)
add_subdirectory(libgame)
add_subdirectory(liblua)
#add_subdirectory(libcrypt)
add_subdirectory(db)
add_subdirectory(game)
#add_subdirectory(auth)
add_subdirectory(quest_compiler)
add_subdirectory(vrunner)
