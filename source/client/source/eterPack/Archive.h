#ifndef FOXFS_ARCHIVE_H
#define FOXFS_ARCHIVE_H

#include <windows.h>
#include <algorithm>
#include <unordered_map>
#include <cstdlib>
#include <cryptopp/crc.h>

#include "FoxFSConfig.h"

namespace FoxFS
{
	
	class Archive
	{
	public:
		enum
		{
			ERROR_OK = 0,
			ERROR_BASE_CODE = 0,
			ERROR_FILE_WAS_NOT_FOUND = ERROR_BASE_CODE + 1,
			ERROR_CORRUPTED_FILE = ERROR_BASE_CODE + 2,
			ERROR_MISSING_KEY = ERROR_BASE_CODE + 3,
			ERROR_MISSING_IV = ERROR_BASE_CODE + 4,
			ERROR_DECRYPTION_HAS_FAILED = ERROR_BASE_CODE + 5,
			ERROR_DECOMPRESSION_FAILED = ERROR_BASE_CODE + 6,
			ERROR_ARCHIVE_NOT_FOUND = ERROR_BASE_CODE + 7,
			ERROR_ARCHIVE_NOT_READABLE = ERROR_BASE_CODE + 8,
			ERROR_ARCHIVE_INVALID = ERROR_BASE_CODE + 9,
			ERROR_ARCHIVE_ACCESS_DENIED = ERROR_BASE_CODE + 10,
			ERROR_KEYSERVER_SOCKET = ERROR_BASE_CODE + 11,
			ERROR_KEYSERVER_CONNECTION = ERROR_BASE_CODE + 12,
			ERROR_KEYSERVER_RESPONSE = ERROR_BASE_CODE + 13,
			ERROR_KEYSERVER_TIMEOUT = ERROR_BASE_CODE + 14,
			ERROR_UNKNOWN = ERROR_BASE_CODE + 15
		};

		struct FileListEntry
		{			
			uint64_t offset;
			uint32_t size;
			uint32_t decompressed;
			uint32_t hash;
			uint32_t name;
		};
		
	public:
		Archive();
		~Archive();
		
		const wchar_t* getFilename() const;
		
		int32_t exists(const char* filename) const;
		uint32_t size(const char* filename) const;
		int32_t get(const char* filename, void* buffer, uint32_t maxsize, uint32_t* outsize) const;
		
		int32_t load(const wchar_t* filename, const void* key, const void* iv);
		void unload();
		
		static uint32_t generateFilenameIndex(const char* filename)
		{
			std::string fn = filename;
			transform(fn.begin(), fn.end(), fn.begin(), tolower);
			for (size_t i = 0; i < fn.length(); ++i)
			{
				if(fn[i] == '\\')
				{
					fn[i] = '/';
				}
			}
			uint32_t index = 0;
			CryptoPP::CRC32 crc;
			crc.CalculateDigest(reinterpret_cast<unsigned char*>(&index), reinterpret_cast<const unsigned char*>(fn.c_str()), fn.length());
			return index;
		}
		
	private:
		std::unordered_map<uint32_t, FileListEntry> files;
		
		unsigned char key[32];
		unsigned char iv[32];
		
		TArchiveHeader header;
		
		HANDLE file;

		wchar_t filename[MAX_PATH + 1];
	};
	
}

#endif