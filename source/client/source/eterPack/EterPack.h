#ifndef __INC_ETERPACKLIB_ETERPACK_H__
#define __INC_ETERPACKLIB_ETERPACK_H__

#include <list>
#include <unordered_map>

#include "../EterBase/MappedFile.h"

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                                  \
                        ((uint32_t)(uint8_t) (ch0       ) | ((uint32_t)(uint8_t) (ch1) <<  8) |     \
                        ((uint32_t)(uint8_t) (ch2) << 16) | ((uint32_t)(uint8_t) (ch3) << 24))
#endif

namespace eterPack
{
	const uint32_t	c_PackCC = MAKEFOURCC('E', 'P', 'K', 'D');
	const uint32_t	c_IndexCC = MAKEFOURCC('E', 'P', 'K', 'D');
	const uint32_t c_Version = 2;
								// FourCC + Version + m_indexCount
	const uint32_t c_HeaderSize = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(int32_t);
}

enum EEterPackTypes
{
	DBNAME_MAX_LEN = 255,
	FILENAME_MAX_LEN = 160,
	FREE_INDEX_BLOCK_SIZE = 32768,
	FREE_INDEX_MAX_SIZE = 512,
	DATA_BLOCK_SIZE = 256,

	COMPRESSED_TYPE_NONE = 0,
	COMPRESSED_TYPE_COMPRESS = 1,
	COMPRESSED_TYPE_SECURITY = 2,
};

#pragma pack(push, 4)
typedef struct SEterPackIndex
{
	int32_t			id;
	char			filename[FILENAME_MAX_LEN + 1];
	uint32_t			filename_crc;
	int32_t			real_data_size;
	int32_t			data_size;
	uint32_t			data_crc;
	int32_t			data_position;
	char			compressed_type;
} TEterPackIndex;
#pragma pack(pop)

typedef std::unordered_map<uint32_t, TEterPackIndex *> TDataPositionMap;
using TFreeIndexList = std::list<TEterPackIndex *>;


class CEterPack;

class CEterFileDict
{
public:
	struct Item
	{
		Item() : pkPack(nullptr), pkInfo(nullptr) {}

		CEterPack*			pkPack;
		TEterPackIndex*		pkInfo;
	};

	enum
	{
		BUCKET_SIZE = 16
	};

	typedef std::unordered_multimap<uint32_t, Item> TDict;

public:
	void InsertItem(CEterPack* pkPack, TEterPackIndex* pkInfo);
	void UpdateItem(CEterPack* pkPack, TEterPackIndex* pkInfo);

	Item* GetItem(uint32_t dwFileNameHash, const char* c_pszFileName);

	const TDict& GetDict() const
	{
		return m_dict;
	}

private:
	TDict m_dict;
};

class CEterPack
{
	public:
		CEterPack();
		virtual ~CEterPack();

		void				Destroy();
		bool				Create(CEterFileDict& rkFileDict, const char * dbname, const char * pathName, bool bReadOnly = true);

	const std::string & GetPathName() const;
	const char * GetDBName() const;

	bool Get(CMappedFile & out_file, const char * filename, LPCVOID * data);
	bool Get2(CMappedFile & out_file, const char * filename, TEterPackIndex * index, LPCVOID * data) const;


	bool Put(const char * filename, const char * sourceFilename, uint8_t packType, const std::string & strRelateMapName);
		bool				Put(const char * filename, LPCVOID data, int32_t len, uint8_t packType);

		bool				Delete(const char * filename);

		bool				Extract();

	int32_t GetFragmentSize() const;

		bool				IsExist(const char * filename);

		TDataPositionMap &	GetIndexMap();

		bool				EncryptIndexFile();
		bool				DecryptIndexFile();

		uint32_t				DeleteUnreferencedData();	// 몇개가 삭제 되었는지 리턴 한다.

		bool				GetNames(std::vector<std::string>* retNames);

	private:
	bool __BuildIndex(CEterFileDict & rkFileDict, bool bOverwrite = false);

	bool CreateIndexFile() const;
		TEterPackIndex *	FindIndex(const char * filename);
		int32_t				GetNewIndexPosition(CFileBase& file);
		TEterPackIndex *	NewIndex(CFileBase& file, const char * filename, int32_t size);
	void WriteIndex(CFileBase & file, TEterPackIndex * index) const;
	static int32_t GetFreeBlockIndex(int32_t size);
		void				PushFreeIndex(TEterPackIndex * index);

	bool CreateDataFile() const;
	static int32_t GetNewDataPosition(CFileBase & file);
	static bool ReadData(CFileBase & file, TEterPackIndex * index, LPVOID data, int32_t maxsize);
	static bool WriteData(CFileBase & file, TEterPackIndex * index, LPCVOID data);
	static bool WriteNewData(CFileBase & file, TEterPackIndex * index, LPCVOID data);

		bool				Delete(TEterPackIndex * pIndex);

	protected:
		CMappedFile				m_file;

		char*					m_file_data;
		uint32_t				m_file_size;

		int32_t					m_indexCount;
		bool					m_bEncrypted;

		char					m_dbName[DBNAME_MAX_LEN+1];
		char					m_indexFileName[MAX_PATH+1];
	std::vector<TEterPackIndex> m_indexData;
		int32_t					m_FragmentSize;
		bool					m_bReadOnly;

	std::unordered_map<uint32_t, uint32_t> m_map_indexRefCount;
		TDataPositionMap		m_DataPositionMap;
		TFreeIndexList			m_FreeIndexList[FREE_INDEX_MAX_SIZE + 1];	// MAX 도 억세스 하므로 + 1 크기만큼 만든다.

		std::string				m_stDataFileName;
		std::string				m_stPathName;
};

class CMakePackLog
{
	public:
		static CMakePackLog& GetSingleton();

	public:
		CMakePackLog();
		~CMakePackLog();

		void SetFileName(const char* c_szFileName);

		void Writef(const char* c_szFormat, ...);
		void Writenf(const char* c_szFormat, ...);
		void Write(const char* c_szBuf);

		void WriteErrorf(const char* c_szFormat, ...);
		void WriteErrornf(const char* c_szFormat, ...);
		void WriteError(const char* c_szBuf);

	void FlushError() const;

	private:
		void __Write(const char* c_szBuf, int32_t nBufLen);
		void __WriteError(const char* c_szBuf, int32_t nBufLen);
	bool __IsLogMode() const;

	private:
		FILE* m_fp;
		FILE* m_fp_err;

		std::string m_stFileName;
		std::string m_stErrorFileName;
};

#endif
