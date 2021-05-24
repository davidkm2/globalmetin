#pragma once

#include <Windows.h>
#include <unordered_map>
#include "../EterBase/Singleton.h"
#include "../EterBase/Stl.h"
#include "../EterBase/MappedFile.h"

#include "../UserInterface/Locale_inc.h"

#ifdef ENABLE_VFS
#include <VFSPack.h>
#include <VFSArchive.h>
#endif

#ifdef ENABLE_FOX_FS

#include <FoxFS.h>
#include "FileSystem.h"


class CFileSystem : public CSingleton<CFileSystem>
{
public:
	CFileSystem();
	virtual ~CFileSystem();

	bool Get(CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData, const char* func, bool ignoreChecks = false);
	void OutputError(int32_t errorCode, bool showAsError, const std::string & stFilename, const char* func);
	static void OutputError(int32_t errorCode, bool showAsError, const std::wstring &  stFilename, const char* func);
	bool GetFromPack(CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData);
	bool GetFromFile(CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData);

	bool isExist(const char* c_szFileName, const char* func, bool ignoreChecks) const;
	bool isExistInPack(const char* c_szFileName, const char* func) const;

	bool LoadArchive(const std::wstring &stName) const;
	bool UnloadArchive(const std::wstring & stName) const;

protected:

	FoxFS::FileSystem * m_pFoxFS;
};

#else

#include "EterPack.h"

class CEterPackManager : public CSingleton<CEterPackManager>
{
	public:
		struct SCache
		{
			uint8_t* m_abBufData;
			uint32_t m_dwBufSize;
		};
	public:
#ifdef ENABLE_VFS
		enum ESearchModes
		{
			SEARCH_FILE,
			SEARCH_PACK
		};

		enum ESearchModes2
		{
			SEARCH_FILE_FIRST,
			SEARCH_PACK_FIRST
		};
#else
		enum ESearchModes
		{
			SEARCH_FILE_FIRST,
			SEARCH_PACK_FIRST
		};
#endif

		using TEterPackList = std::list<CEterPack *>;
		typedef std::unordered_map<std::string, CEterPack*, stringhash> TEterPackMap;

	public:
		CEterPackManager();
		virtual ~CEterPackManager();

		void SetCacheMode();

		void LoadStaticCache(const char* c_szFileName);

		void SetSearchMode(bool bPackFirst);
		int32_t GetSearchMode() const;

		//THEMIDA
		bool Get(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData);

		//THEMIDA
		bool GetFromPack(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData);

		//THEMIDA
		bool GetFromFile(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData);
		bool isExist(const char * c_szFileName);
		bool isExistInPack(const char * c_szFileName);

		bool RegisterPack(const char * c_szName, const char * c_szDirectory, const uint8_t* c_pbIV = nullptr);
		void RegisterRootPack(const char * c_szName, const char* c_szKey = nullptr);
		bool RegisterPackWhenPackMaking(const char * c_szName, const char * c_szDirectory, CEterPack* pPack);

#ifdef ENABLE_VFS
		void LoadRegistiredPacks();
		bool InitializeVFS();
#endif
		// 메모리에 매핑된 팩들 가운데, 정리해야할 것들 정리.
	public:
		static void ArrangeMemoryMappedPack();

	protected:
		static int32_t ConvertFileName(const char * c_szFileName, std::string & rstrFileName); // StringPath std::string 버전
		static bool CompareName(const char * c_szDirectoryName, uint32_t iLength, const char * c_szFileName);

		CEterPack* FindPack(const char* c_szPathName);

		SCache* __FindCache(uint32_t dwFileNameHash);
		void	__ClearCacheMap();

	protected:
		bool					m_bTryRelativePath;
		bool					m_isCacheMode;
		int32_t						m_iSearchMode;

		CEterFileDict			m_FileDict;
		CEterPack				m_RootPack;
		TEterPackList			m_PackList;
		TEterPackMap			m_PackMap;
		TEterPackMap			m_DirPackMap;

		std::unordered_map<uint32_t, SCache> m_kMap_dwNameKey_kCache;

		CRITICAL_SECTION		m_csFinder;
#ifdef ENABLE_VFS
		VFS::CVFSPack					m_vfsManager;
		std::vector <VFS::CVFSArchive*> m_vfsArchives;
#endif
};
#endif
