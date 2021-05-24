cmake_minimum_required(VERSION 2.6)
project(FoxFS)

include_directories(${PROJECT_SOURCE_DIR}/extern/include)
link_directories(${PROJECT_SOURCE_DIR}/extern/lib)

add_library(vs_files/lz4/lz4 STATIC		${PROJECT_SOURCE_DIR}/source/lz4/lz4.h
								${PROJECT_SOURCE_DIR}/source/lz4/lz4hc.h
								${PROJECT_SOURCE_DIR}/source/lz4/lz4.c
								${PROJECT_SOURCE_DIR}/source/lz4/lz4hc.c
)
add_library(vs_files/xxhash/xxhash STATIC	${PROJECT_SOURCE_DIR}/source/xxhash/xxhash.h
								${PROJECT_SOURCE_DIR}/source/xxhash/xxhash.c
)
add_library(vs_files/FoxFS/FoxFS STATIC		${PROJECT_SOURCE_DIR}/source/FoxFS/FoxFS.h
								${PROJECT_SOURCE_DIR}/source/lz4/lz4.h
								${PROJECT_SOURCE_DIR}/source/lz4/lz4hc.h
								${PROJECT_SOURCE_DIR}/source/xxhash/xxhash.h
								${PROJECT_SOURCE_DIR}/source/FoxFS/FoxFS.cc
								${PROJECT_SOURCE_DIR}/source/FoxFS/config.h
								${PROJECT_SOURCE_DIR}/source/FoxFS/FileSystem.h
								${PROJECT_SOURCE_DIR}/source/FoxFS/Archive.h
								${PROJECT_SOURCE_DIR}/source/FoxFS/FileSystem.cc
								${PROJECT_SOURCE_DIR}/source/FoxFS/Archive.cc
)
add_executable(vs_files/FoxFSArchiver/FoxFSArchiver	${PROJECT_SOURCE_DIR}/source/lz4/lz4.h
								${PROJECT_SOURCE_DIR}/source/lz4/lz4hc.h
								${PROJECT_SOURCE_DIR}/source/xxhash/xxhash.h
								${PROJECT_SOURCE_DIR}/source/FoxFS/config.h
								${PROJECT_SOURCE_DIR}/source/archiver/ArchiveWriter.h
								${PROJECT_SOURCE_DIR}/source/archiver/ArchiveWriter.cc
								${PROJECT_SOURCE_DIR}/source/archiver/main.cc
)
add_executable(vs_files/FoxFSServer/FoxFSServer	${PROJECT_SOURCE_DIR}/source/server/main.cc
								${PROJECT_SOURCE_DIR}/source/server/Application.h
								${PROJECT_SOURCE_DIR}/source/server/Application.cc
)
target_link_libraries(vs_files/FoxFSArchiver/FoxFSArchiver Ws2_32 cryptopp lz4 xxhash tbb)
target_link_libraries(vs_files/FoxFSServer/FoxFSServer Ws2_32 cryptopp lz4 xxhash tbb)
