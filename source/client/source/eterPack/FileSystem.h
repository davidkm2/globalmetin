#ifndef FOXFS_FILESYSTEM_H
#define FOXFS_FILESYSTEM_H

#include <string>
#include <map>

#include "Archive.h"

#include <windows.h>      
#include <iphlpapi.h>   


namespace FoxFS
{
	
	class FileSystem
	{
		struct KeyInfo
		{
			unsigned char key[32];
			unsigned char iv[32];
		};
		
	public:
		FileSystem();
		~FileSystem();
				
		int32_t load(const wchar_t* filename);

		int32_t unload(const wchar_t* filename);
		
		uint32_t size(const char* filename) const;

		int32_t exists(const char* filename) const;

		int32_t get(const char* filename, void* buffer, uint32_t maxsize, uint32_t* outsize) const;

	private:
		std::map<std::basic_string<wchar_t>, Archive*> archives;
		
		std::map<std::string, KeyInfo> keys;
	
		mutable CRITICAL_SECTION mutex;

	};
	
}

#endif