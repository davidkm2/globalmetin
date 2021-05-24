#ifndef FOXFS_ARCHIVEWRITER_H
#define FOXFS_ARCHIVEWRITER_H

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
	#include <windows.h>
#else
	#include <limits.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <fcntl.h>
#endif

class ArchiveWriter
{
public:
	ArchiveWriter();
	~ArchiveWriter();
	
	bool create(const char* filename, const char* keyfile = 0);
	void close();
	
	bool add(const char* filename, unsigned int decompressed, unsigned int compressed, unsigned int hash, const void* data);

private:
	unsigned char key[32];
	unsigned char iv[32];

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
	CRITICAL_SECTION mutex;
	HANDLE file;
	HANDLE keys;
#else
	pthread_mutex_t mutex;
	int file;
	int keys;
#endif
};

#endif