cmake_minimum_required(VERSION 3.2)
project(FoxNet)

# Output folder
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../../Bin/Utilities)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

add_definitions(-DUSE_BOOST_ASIO)

set(FOXNET_HEADERS	include/FoxNet.h
					include/NetService.h
					include/NetServer.h
					include/NetClient.h
					include/NetPeer.h
)
set(FOXNET_SOURCES	src/NetService.cpp
					src/NetServer.cpp
					src/NetClient.cpp
					src/NetPeer.cpp
)

add_library(${PROJECT_NAME} STATIC ${FOXNET_HEADERS} ${FOXNET_SOURCES})

install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
)
install(FILES ${FOXNET_HEADERS} DESTINATION include)
