#include "StdAfx.h"
#include "EterPackManager.h"

#include <io.h>
#include <cassert>
#include <XORstr.h>

#include "../EterBase/Debug.h"
#include "../EterBase/CRC32.h"
#include "../AntiCheatLib/CheatQueueManager.h"


#ifndef ENABLE_FOX_FS

CEterPackManager::CEterPackManager()
{
	InitializeCriticalSection(&m_csFinder);
}

CEterPackManager::~CEterPackManager()
{
	__ClearCacheMap();

	TEterPackMap::iterator i = m_PackMap.begin();
	TEterPackMap::iterator e = m_PackMap.end();
	while (i != e)
	{
		delete i->second;
		++i;
	}
	DeleteCriticalSection(&m_csFinder);
}

#ifdef ENABLE_VFS
const char* white_file_list[] = { "logininfo.xml", "mark\10_0.tga" "mark\250_0.tga" };

bool isWhiteFile(const char* c_szFileName)
{
	for (int i = 0; i < ARRAYSIZE(white_file_list); i++)
	{
		if (stricmp(c_szFileName, white_file_list[i]) == 0)
		{
			return true;
		}
	}
	return false;
}

const char* white_file_list_ext[] = { "xml", "tga", "png", "bmp", "mp3", "jpg" };

bool isWhiteFileExt(const char* c_szFileName)
{
	for (int i = 0; i < ARRAYSIZE(white_file_list_ext); i++)
	{
		std::string ext = CFileNameHelper::GetExtension(std::string(c_szFileName));
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		std::string whiteExt = std::string(white_file_list_ext[i]);

		if (ext == whiteExt)
			return true;
	}
	return false;
}
#endif

CEterPack* CEterPackManager::FindPack(const char* c_szPathName)
{
	std::string strFileName;

	if (0 == ConvertFileName(c_szPathName, strFileName))
		return &m_RootPack;


	for (auto & itor : m_DirPackMap)
	{
		const std::string & c_rstrName = itor.first;
		CEterPack * pEterPack = itor.second;

		if (CompareName(c_rstrName.c_str(), c_rstrName.length(), strFileName.c_str()))
			return pEterPack;
	}


	return nullptr;
}

void CEterPackManager::SetCacheMode()
{
	m_isCacheMode=true;
}


// StringPath std::string 버전
int32_t CEterPackManager::ConvertFileName(const char * c_szFileName, std::string & rstrFileName)
{
	rstrFileName = c_szFileName;
	stl_lowers(rstrFileName);

	int32_t iCount = 0;
	for (auto & i : rstrFileName)
	{
		if (i == '/')
			++iCount;
		else if (i == '\\')
		{
			i = '/';
			++iCount;
		}
	}

	return iCount;
}

bool CEterPackManager::CompareName(const char * c_szDirectoryName, uint32_t /*dwLength*/, const char * c_szFileName)
{
	const char * c_pszSrc = c_szDirectoryName;
	const char * c_pszCmp = c_szFileName;

	while (*c_pszSrc)
	{
		if (*(c_pszSrc++) != *(c_pszCmp++))
			return false;

		if (!*c_pszCmp)
			return false;
	}

	return true;
}

void CEterPackManager::LoadStaticCache(const char* c_szFileName)
{
	if (!m_isCacheMode)
		return;

	std::string strFileName;
	if (0 == ConvertFileName(c_szFileName, strFileName))
		return;

	uint32_t dwFileNameHash = GetCRC32(strFileName.c_str(), strFileName.length());

	auto f = m_kMap_dwNameKey_kCache.find(dwFileNameHash);
	if (f != m_kMap_dwNameKey_kCache.end())
		return;

	CMappedFile kMapFile;
	const void* c_pvData;
	if (!Get(kMapFile, c_szFileName, &c_pvData))
		return;

	SCache kNewCache;
	kNewCache.m_dwBufSize = kMapFile.Size();
	kNewCache.m_abBufData = new uint8_t[kNewCache.m_dwBufSize];
	memcpy(kNewCache.m_abBufData, c_pvData, kNewCache.m_dwBufSize);
	m_kMap_dwNameKey_kCache.emplace(dwFileNameHash, kNewCache);
}

CEterPackManager::SCache* CEterPackManager::__FindCache(uint32_t dwFileNameHash)
{
	auto f = m_kMap_dwNameKey_kCache.find(dwFileNameHash);
	if (m_kMap_dwNameKey_kCache.end() != f)
		return &f->second;
		return nullptr;
}

void	CEterPackManager::__ClearCacheMap()
{
	for (auto & i : m_kMap_dwNameKey_kCache)
		delete[] i.second.m_abBufData;

	m_kMap_dwNameKey_kCache.clear();
}

struct TimeChecker
{
	explicit TimeChecker(const char * name) : name(name) { baseTime = timeGetTime(); }
	~TimeChecker() { printf("load %s (%lu)\n", name, timeGetTime() - baseTime); }

	const char* name;
	uint32_t baseTime;
};

bool CEterPackManager::Get(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData)
{
#ifdef ENABLE_VFS
	if (m_iSearchMode == SEARCH_FILE)
	{
		if (GetFromFile(rMappedFile, c_szFileName, pData))
		{
			return true;
		}

		return GetFromPack(rMappedFile, c_szFileName, pData);
	}
	else
	{
		if (isExistInPack(c_szFileName))
		{
			return GetFromPack(rMappedFile, c_szFileName, pData);
		}
		else if (isExist(c_szFileName))
		{
			return GetFromFile(rMappedFile, c_szFileName, pData);
		}
	}
	return false;
#else
	if (m_iSearchMode == SEARCH_FILE_FIRST)
	{
		if (GetFromFile(rMappedFile, c_szFileName, pData))
			return true;

		return GetFromPack(rMappedFile, c_szFileName, pData);
	}

	if (GetFromPack(rMappedFile, c_szFileName, pData))
		return true;

	return GetFromFile(rMappedFile, c_szFileName, pData);
#endif
}

struct FinderLock
{
	explicit FinderLock(CRITICAL_SECTION & cs) : p_cs(&cs) { EnterCriticalSection(p_cs); }

	~FinderLock() { LeaveCriticalSection(p_cs); }

	CRITICAL_SECTION * p_cs;
};

bool CEterPackManager::GetFromPack(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData)
{
	assert(c_szFileName);

	FinderLock lock(m_csFinder);

#ifdef ENABLE_VFS
	auto archives = m_vfsManager.GetArchives();
	auto regarchives = m_vfsManager.GetRegisteredArchives();
	if (archives.size() == 0 && regarchives.size() == 0)
	{
		TraceError("Any archive not load yet");
		return false;
	}

	for (const auto& archive : archives)
	{
		if (archive && archive.get())
		{
			auto stream = archive->Open(m_vfsManager.ToWstring(c_szFileName));

			if (stream && stream.get())
			{
				auto dwSize = static_cast<uint32_t>(stream->GetSize());

				uint8_t* pbData = new uint8_t[dwSize + 1];
				memcpy(pbData, stream->GetData(), dwSize);

				pbData[stream->GetSize()] = 0;
				*pData = pbData;
				rMappedFile.Link(dwSize, pbData);
				return true;
			}
		}
	}

//	TraceError("GetFromPack not found: %s", c_szFileName);
	return false;
#endif

	static std::string strFileName;

	if (0 == ConvertFileName(c_szFileName, strFileName))
		return m_RootPack.Get(rMappedFile, strFileName.c_str(), pData);


	uint32_t dwFileNameHash = GetCRC32(strFileName.c_str(), strFileName.length());
	SCache * pkCache = __FindCache(dwFileNameHash);

	if (pkCache)
	{
		rMappedFile.Link(pkCache->m_dwBufSize, pkCache->m_abBufData);
		return true;
	}

	CEterFileDict::Item * pkFileItem = m_FileDict.GetItem(dwFileNameHash, strFileName.c_str());

	if (pkFileItem)
		if (pkFileItem->pkPack)
		{
			bool r = pkFileItem->pkPack->Get2(rMappedFile, strFileName.c_str(), pkFileItem->pkInfo, pData);
			return r;
		}

#ifdef _DEBUG
	TraceError("CANNOT_FIND_PACK_FILE [%s]", strFileName.c_str());
#endif

	return false;
}

const time_t g_tCachingInterval = 10; // 10초
void CEterPackManager::ArrangeMemoryMappedPack() {}

bool CEterPackManager::GetFromFile (CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData)
{
#ifndef _DEBUG
	if (!isExistInPack(c_szFileName))
	{
		const char* pcExt = strchr(c_szFileName, '.');
		if (pcExt)
		{
			if ((!strstr(c_szFileName, XOR("/")) || !strstr(c_szFileName, XOR("\\"))) &&
				(!strcmp(pcExt, XOR(".py")) || !strcmp(pcExt, XOR(".pyc")) || !strcmp(pcExt, XOR(".pyo"))))
			{
				CCheatDetectQueueMgr::Instance().AppendDetection(CHEAT_PY_LOAD, 0, c_szFileName);
				return false;
			}
		}
	}
#endif

	return rMappedFile.Create (c_szFileName, pData, 0, 0) ? true : false;
}

bool CEterPackManager::isExistInPack(const char * c_szFileName)
{
#ifdef ENABLE_VFS
	assert(c_szFileName);

	if (m_vfsManager.GetArchives().begin() == m_vfsManager.GetArchives().end())
	{
		TraceError("No archives loaded");
		return false;
	}

//	TraceError("isExistInPack: %s", c_szFileName);
	for (const auto& archive : m_vfsManager.GetArchives())
	{
		if (archive->Exists(c_szFileName))
		{
			return true;
		}
	}

//	TraceError("%s not exist", c_szFileName);
	return false;
#endif

	std::string strFileName;

	if (0 == ConvertFileName(c_szFileName, strFileName))
		return m_RootPack.IsExist(strFileName.c_str());


	uint32_t dwFileNameHash = GetCRC32(strFileName.c_str(), strFileName.length());
	CEterFileDict::Item * pkFileItem = m_FileDict.GetItem(dwFileNameHash, strFileName.c_str());

	if (pkFileItem)
	{
		if (pkFileItem->pkPack)
			return pkFileItem->pkPack->IsExist(strFileName.c_str());
	}


	// NOTE : 매치 되는 팩이 없다면 false - [levites]
	return false;
}

bool CEterPackManager::isExist(const char * c_szFileName)
{
#ifdef ENABLE_VFS
	if (m_iSearchMode == SEARCH_PACK)
	{
		if (isWhiteFile(c_szFileName) || isWhiteFileExt(c_szFileName))
		{
			return isExistInPack(c_szFileName) || (_access(c_szFileName, 0) == 0);
		}
		return isExistInPack(c_szFileName);
	}

	if (_access(c_szFileName, 0) == 0)
		return true;

	return isExistInPack(c_szFileName);
#endif
	if (m_iSearchMode == SEARCH_PACK_FIRST)
	{
		if (isExistInPack(c_szFileName))
			return true;

		return _access(c_szFileName, 0) == 0;
	}

	if (_access(c_szFileName, 0) == 0)
		return true;

	return isExistInPack(c_szFileName);
}


void CEterPackManager::RegisterRootPack(const char * c_szName, const char* c_szKey)
{
#ifdef ENABLE_VFS
	assert(c_szName);

	auto key = m_vfsManager.ConvertKeyFromAscii(c_szKey);
	if (key.empty())
		return;

	m_vfsManager.SetArchiveKey(m_vfsManager.ToWstring(c_szName), key.data());

	if (!m_vfsManager.LoadArchive(m_vfsManager.ToWstring(c_szName)))
	{
		TraceError("Cannot load %s", c_szName);
	}
	return;
#endif

	if (!m_RootPack.Create(m_FileDict, c_szName, ""))
		TraceError("%s: Pack file does not exist", c_szName);
}

bool CEterPackManager::RegisterPackWhenPackMaking(const char * c_szName, const char * c_szDirectory, CEterPack* pPack)
{
	m_PackMap.emplace(c_szName, pPack);
	m_PackList.push_front(pPack);

	m_DirPackMap.emplace(c_szDirectory, pPack);
	return true;
}


bool CEterPackManager::RegisterPack(const char * c_szName, const char * c_szDirectory, const uint8_t* c_pbIV)
{
#ifdef ENABLE_VFS
	assert(c_szName);

	auto key = m_vfsManager.ConvertKeyFromAscii(reinterpret_cast<const char*>(c_pbIV));
	if (key.empty())
		return false;

	m_vfsManager.SetArchiveKey(m_vfsManager.ToWstring(c_szName), key.data());
	m_vfsManager.RegisterArchive(m_vfsManager.ToWstring(c_szName), m_vfsManager.ToWstring(c_szDirectory));

	return true;
#endif

	CEterPack * pEterPack;
	{
		auto itor = m_PackMap.find(c_szName);

		if (m_PackMap.end() == itor)
		{
			bool bReadOnly = true;

			pEterPack = new CEterPack;
			if (pEterPack->Create(m_FileDict, c_szName, c_szDirectory, bReadOnly))
				m_PackMap.emplace(c_szName, pEterPack);
			else
			{
#ifdef _DEBUG
				Tracef("The eterPack doesn't exist [%s]\n", c_szName);
#endif
				delete pEterPack;
				pEterPack = nullptr;
				return false;
			}
		}
		else
			pEterPack = itor->second;
	}

	if (c_szDirectory && c_szDirectory[0] != '*')
	{
		auto itor = m_DirPackMap.find(c_szDirectory);
		if (m_DirPackMap.end() == itor)
		{
			m_PackList.push_front(pEterPack);
			m_DirPackMap.emplace(c_szDirectory, pEterPack);
		}
	}

	return true;
}

void CEterPackManager::SetSearchMode(bool bPackFirst)
{
	m_iSearchMode = bPackFirst ? SEARCH_PACK_FIRST : SEARCH_FILE_FIRST;
}

int32_t CEterPackManager::GetSearchMode() const
{
	return m_iSearchMode;
}

#endif

#ifdef ENABLE_VFS
void CEterPackManager::LoadRegistiredPacks()
{
	m_vfsManager.LoadRegistiredArchives();
}

bool CEterPackManager::InitializeVFS()
{
	return m_vfsManager.InitializeVFSPack();
}
#endif
