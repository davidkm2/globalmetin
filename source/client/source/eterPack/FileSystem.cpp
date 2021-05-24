#include "StdAfx.h"
#include <FoxFS.h>
#include "FileSystem.h"
#include "Archive.h"
#include <algorithm>

namespace FoxFS
{
	FileSystem::FileSystem()
	{
		InitializeCriticalSection(&mutex);
	}

	FileSystem::~FileSystem()
	{
		for (auto iter = archives.begin(); iter != archives.end(); ++iter)
		{
			delete iter->second;
		}

		DeleteCriticalSection(&mutex);

	}

	int32_t FileSystem::load(const wchar_t* filename)
	{
		EnterCriticalSection(&mutex);

		int32_t r = Archive::ERROR_OK;
		std::basic_string<wchar_t> fn = filename;
		transform(fn.begin(), fn.end(), fn.begin(), tolower);

		auto iter = archives.find(fn);
		if (iter == archives.end())
		{
			Archive* a = new Archive();
			if (a)
			{
				char tmpname[MAX_PATH + 1] = {0};
				wcstombs(tmpname, filename, wcslen(filename));

				auto keyiter = keys.find(tmpname);
				void *key = nullptr, *iv = nullptr;
				if (keyiter != keys.end())
				{
					key = keyiter->second.key;
					iv = keyiter->second.iv;
				}

				if ((r = a->load(filename, key, iv)) == Archive::ERROR_OK)
				{
					archives.emplace(fn, a);
				}
				else
				{
					delete a;
				}
			}
		}

		LeaveCriticalSection(&mutex);

		return r;
	}

	int32_t FileSystem::unload(const wchar_t* filename)
	{
		EnterCriticalSection(&mutex);
		
		std::basic_string<wchar_t> fn = filename;
		transform(fn.begin(), fn.end(), fn.begin(), tolower);

		auto iter = archives.find(fn);

		int32_t r = Archive::ERROR_ARCHIVE_NOT_FOUND;
		if (iter != archives.end())
		{
			delete iter->second;
			archives.erase(iter);
			r = Archive::ERROR_OK;
		}

		LeaveCriticalSection(&mutex);

		return r;
	}

	uint32_t FileSystem::size(const char* filename) const
	{
		EnterCriticalSection(&mutex);

		uint32_t r = 0;
		for (const auto & elem : archives)
		{
			if ((r = elem.second->size(filename)) > 0)
			{
				break;
			}
		}

		LeaveCriticalSection(&mutex);
		return r;
	}

	int32_t FileSystem::exists(const char* filename) const
	{
		EnterCriticalSection(&mutex);

		int32_t r = Archive::ERROR_FILE_WAS_NOT_FOUND;
		for (const auto & elem : archives)
		{
			if ((r = elem.second->exists(filename)) == Archive::ERROR_OK)
			{
				break;
			}
		}

		LeaveCriticalSection(&mutex);
		return r;
	}

	int32_t FileSystem::get(const char* filename, void* buffer, uint32_t maxsize, uint32_t* outsize) const
	{
		EnterCriticalSection(&mutex);

		int32_t r = Archive::ERROR_FILE_WAS_NOT_FOUND;

		for (const auto & elem : archives)
		{
			if ((r = elem.second->get(filename, buffer, maxsize, outsize)) == Archive::ERROR_OK)
				break;
		}

		LeaveCriticalSection(&mutex);

		return r;
	}
}
