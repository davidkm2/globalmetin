#include "StdAfx.h"
#include <FoxFS.h>
#include "Archive.h"

#include <cryptopp/modes.h>
#include <cryptopp/aes.h>

#include "../AntiCheatLib/xxhash.h"

#include <lz4/lib/lz4.h>
#include <lz4/lib/lz4hc.h>

namespace FoxFS
{
#ifdef MIN
#undef MIN
#endif

	template <typename typeA, typename typeB>
	typeA MIN(typeA a, typeB b)
	{
		return a < b ? a : b;
	}

#ifdef MAX
#undef MAX
#endif

	template <typename typeA, typename typeB>
	typeA MAX(typeA a, typeB b)
	{
		return a > b ? a : b;
	}

#ifdef MINMAX
#undef MINMAX
#endif

	template <typename typeMin, typename typeValue, typename typeMax>
	typeValue MINMAX(typeMin min, typeValue value, typeMax max)
	{
		return ((value) < (min) ? (min) : ((max) < (value) ? (max) : (value)));
	}

	Archive::Archive() :
		file(INVALID_HANDLE_VALUE)

	{
	}

	Archive::~Archive()
	{
		unload();
	}

	const wchar_t* Archive::getFilename() const
	{
		return filename;
	}

	int32_t Archive::exists(const char* filename) const
	{
		uint32_t index = generateFilenameIndex(filename);

		int32_t r = ERROR_FILE_WAS_NOT_FOUND;
		auto iter = files.find(index);
		if (iter != files.end())
		{
			r = ERROR_OK;
		}
		return r;
	}

	uint32_t Archive::size(const char* filename) const
	{
		auto index = generateFilenameIndex(filename);

		uint32_t r = 0;
		auto iter = files.find(index);
		if (iter != files.end())
		{
			r = iter->second.decompressed;
		}
		return r;
	}

	int32_t Archive::get(const char* filename, void* buffer, uint32_t maxsize, uint32_t* outsize) const
	{
		auto index = generateFilenameIndex(filename);

		int32_t r = ERROR_FILE_WAS_NOT_FOUND;
		auto iter = files.find(index);
		if (iter != files.end())
		{
			if (iter->second.size == iter->second.decompressed)
			{
				uint32_t len = MIN(maxsize, iter->second.size);

				LARGE_INTEGER m;
				m.QuadPart = iter->second.offset;
				SetFilePointerEx(file, m, nullptr, FILE_BEGIN);

				auto dwRead = 0UL;
				ReadFile(file, buffer, len, &dwRead, nullptr);

				try
				{
					CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decoder(key, 32, iv);
					decoder.ProcessData(reinterpret_cast<unsigned char*>(buffer), reinterpret_cast<unsigned char*>(buffer), len);
				}
				catch (...)
				{
					return ERROR_DECRYPTION_FAILED;
				}
				if (len == iter->second.size && XXH32(reinterpret_cast<char*>(buffer), len, FOXFS_MAGIC) != iter->second.hash)
				{
					return ERROR_CORRUPTED_FILE;
				}
				if (outsize)
				{
					*outsize = len;
				}
			}
			else
			{
				std::vector<unsigned char> tmp(iter->second.size + iter->second.decompressed);

				LARGE_INTEGER m;
				m.QuadPart = iter->second.offset;
				SetFilePointerEx(file, m, 0, FILE_BEGIN);

				auto dwRead = 0UL;
				ReadFile(file, &tmp[0], iter->second.size, &dwRead, nullptr);

				try
				{
					CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decoder(key, 32, iv);
					decoder.ProcessData(&tmp[0], &tmp[0], iter->second.size);
				}
				catch (...)
				{
					return ERROR_DECRYPTION_FAILED;
				}
				if(LZ4_decompress_fast(reinterpret_cast<char*>(&tmp[0]), reinterpret_cast<char*>(&tmp[iter->second.size]), iter->second.decompressed) != iter->second.size)
				{
					return ERROR_DECOMPRESSION_FAILED;
				}
				if(XXH32(reinterpret_cast<char*>(&tmp[iter->second.size]), iter->second.decompressed, FOXFS_MAGIC) != iter->second.hash)
				{
					return ERROR_CORRUPTED_FILE;
				}
				memcpy(buffer, &tmp[iter->second.size], MIN(maxsize, iter->second.decompressed));
				if (outsize)
				{
					*outsize = MIN(maxsize, iter->second.decompressed);
				}
			}
			r = ERROR_OK;
		}
		return r;
	}

	int32_t Archive::load(const wchar_t* filename, const void* key, const void* iv)
	{
		unload();

		file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE)
		{
			switch (GetLastError())
			{
			case ERROR_ACCESS_DENIED: return ERROR_ARCHIVE_ACCESS_DENIED;
			case ERROR_FILE_NOT_FOUND: return ERROR_ARCHIVE_NOT_FOUND;
			case ERROR_TOO_MANY_OPEN_FILES: return ERROR_ARCHIVE_NOT_READABLE;
			default: return ERROR_ARCHIVE_ACCESS_DENIED;
			}
		}
		wcscpy_s(this->filename, filename);
		auto dwRead = 0UL;
		ReadFile(file, &header, sizeof(header), &dwRead, nullptr);
		auto size = GetFileSize(file, nullptr);

		if (header.magic != FOXFS_MAGIC)
		{
			unload();
			return ERROR_ARCHIVE_INVALID;
		}
		memcpy(this->key, header.key, 32);
		memcpy(this->iv, header.iv, 32);
		if (key)
		{
			for (int32_t i = 0; i < 4; ++i)
			{
				reinterpret_cast<uint64_t*>(this->key)[i] ^= reinterpret_cast<const uint64_t*>(key)[i];
			}
		}
		if (iv)
		{
			for (int32_t i = 0; i < 4; ++i)
			{
				reinterpret_cast<uint64_t*>(this->iv)[i] ^= reinterpret_cast<const uint64_t*>(iv)[i];
			}
		}
		uint32_t offset = sizeof(header);
		while (offset < size)
		{
			TArchiveEntry entry;

			LARGE_INTEGER m;
			m.QuadPart = offset;
			SetFilePointerEx(file, m, nullptr, FILE_BEGIN);
			ReadFile(file, &entry, sizeof(entry), &dwRead, nullptr);

			offset = entry.offset + entry.size;
			FileListEntry listentry;
			listentry.offset = entry.offset;
			listentry.size = entry.size;
			listentry.decompressed = entry.decompressed;
			listentry.hash = entry.hash;
			listentry.name = entry.name;
			files.emplace(std::map<uint32_t, FileListEntry>::value_type(listentry.name, listentry));
		}
		return ERROR_OK;
	}

	void Archive::unload()
	{
		files.clear();
		memset(filename, 0, sizeof(filename));
		memset(&header, 0, sizeof(header));
		memset(key, 0, sizeof(key));
		memset(iv, 0, sizeof(iv));

		if (file != INVALID_HANDLE_VALUE)
		{
			CloseHandle(file);
			file = INVALID_HANDLE_VALUE;
		}
	}
}
