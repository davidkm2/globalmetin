#include "StdAfx.h"
#include "EterPack.h"
#include <iostream>
#include <fstream>

#include <Windows.h>
#include <mmsystem.h>
#include <cassert>
#include <iterator>

#include "../EterBase/Utils.h"
#include "../EterBase/Debug.h"
#include "../EterBase/CRC32.h"
#include "../EterBase/file_ptr.h"

inline void inlinePathCreate(const char* path)
{
	char dir[MAX_PATH];
	const char *p, *k;

	p = path + 3;

	while ((k = strchr(p, '/')))
	{
		memset(dir, 0, sizeof(dir));
		strncpy_s(dir, path, k - path);
		CreateDirectory(dir, nullptr);
		p = k + 1;
	}
}

inline void inlineConvertPackFilename(char* name)
{
	char *p = name;

	while (*p)
	{
		if (*p == '\\')
			*p = '/';
		else
			*p = (int32_t)tolower(*p);
		p++;
	}
}


void CMakePackLog::SetFileName(const char* c_szFileName)
{
	m_stFileName=c_szFileName;
	m_stFileName +=".log";
	m_stErrorFileName = c_szFileName;
	m_stErrorFileName += ".err";
}

CMakePackLog& CMakePackLog::GetSingleton()
{
	static CMakePackLog s_kMakePackLog;
	return s_kMakePackLog;
}

CMakePackLog::CMakePackLog()
{
	m_fp=nullptr;
	m_fp_err = nullptr;
}

CMakePackLog::~CMakePackLog()
{
	if (nullptr!=m_fp)
	{
		fclose(m_fp);
		m_fp=nullptr;
	}
	if (nullptr != m_fp_err)
	{
		fclose(m_fp_err);
		m_fp_err = nullptr;
	}
}

bool CMakePackLog::__IsLogMode() const
{
	if (m_stFileName.empty())
		return false;
	return true;
}

void CMakePackLog::Writef(const char* c_szFormat, ...)
{
	if (!__IsLogMode())
		return;

	va_list args;
	va_start(args, c_szFormat);

	char szBuf[1024];
	int32_t nBufLen = _vsnprintf_s(szBuf, sizeof(szBuf), c_szFormat, args);
	szBuf[nBufLen] = '\0';
	__Write(szBuf, nBufLen);
}

void CMakePackLog::Writenf(const char* c_szFormat, ...)
{
	if (!__IsLogMode())
		return;

	va_list args;
	va_start(args, c_szFormat);

	char szBuf[1024+1];
	int32_t nBufLen = _vsnprintf_s(szBuf, sizeof(szBuf)-1, c_szFormat, args);
	if (nBufLen > 0)
	{
		szBuf[nBufLen++] = '\n';
		szBuf[nBufLen] = '\0';
	}
	__Write(szBuf, nBufLen);
}

void CMakePackLog::Write(const char* c_szBuf)
{
	if (!__IsLogMode())
		return;

	__Write(c_szBuf, strlen(c_szBuf)+1);
}

void CMakePackLog::__Write(const char* c_szBuf, int32_t nBufLen)
{
	if (!__IsLogMode())
		return;

	fopen_s(&m_fp, m_stFileName.c_str(), "w");

	fwrite(c_szBuf, nBufLen, 1, m_fp);

	printf("%s", c_szBuf);
}



void CMakePackLog::WriteErrorf(const char* c_szFormat, ...)
{
	if (!__IsLogMode())
		return;

	va_list args;
	va_start(args, c_szFormat);

	char szBuf[1024];
	int32_t nBufLen = _vsnprintf_s(szBuf, sizeof(szBuf), c_szFormat, args);
	szBuf[nBufLen] = '\0';
	__WriteError(szBuf, nBufLen);
}

void CMakePackLog::WriteErrornf(const char* c_szFormat, ...)
{
	if (!__IsLogMode())
		return;

	va_list args;
	va_start(args, c_szFormat);

	char szBuf[1024+1];
	int32_t nBufLen = _vsnprintf_s(szBuf, sizeof(szBuf)-1, c_szFormat, args);
	if (nBufLen > 0)
	{
		szBuf[nBufLen++] = '\n';
		szBuf[nBufLen] = '\0';
	}
	__WriteError(szBuf, nBufLen);
}

void CMakePackLog::WriteError(const char* c_szBuf)
{
	if (!__IsLogMode())
		return;

	__WriteError(c_szBuf, strlen(c_szBuf)+1);
}

void CMakePackLog::__WriteError(const char* c_szBuf, int32_t nBufLen)
{
	if (!__IsLogMode())
		return;

	fopen_s(&m_fp_err, m_stErrorFileName.c_str(), "w");

	fwrite(c_szBuf, nBufLen, 1, m_fp_err);

	printf("Error: %s", c_szBuf);
}

void CMakePackLog::FlushError() const
{
	std::wifstream iFile(m_stErrorFileName.c_str());
    std::istream_iterator <std::wstring, wchar_t> iit(iFile);
    std::istream_iterator <std::wstring, wchar_t> eos;

    std::vector <std::wstring> vText;

    std::copy (iit, eos, std::back_inserter(vText));

    std::ostream_iterator <std::wstring, wchar_t, std::char_traits <wchar_t> > oit(std::wcout);

    std::sort (vText.begin(), vText.end());
    std::copy (vText.begin(), vText.end(), oit);
}
#pragma optimize("", off)
///////////////////////////////////////////////////////////////////////////////
CEterPack::CEterPack() :
	m_indexCount(0), m_FragmentSize(0), m_bEncrypted(false), m_bReadOnly(false)
{
	m_indexData.clear();
}

CEterPack::~CEterPack()
{
	Destroy();
}

void CEterPack::Destroy()
{
	m_bReadOnly = false;
	m_bEncrypted = false;
	m_indexCount = 0;
	m_DataPositionMap.clear();

	for (auto & i : m_FreeIndexList)
		i.clear();

	m_indexData.clear();

	m_FragmentSize = 0;

	memset(m_dbName, 0, sizeof(m_dbName));
	memset(m_indexFileName, 0, sizeof(m_indexFileName));
}

const std::string & CEterPack::GetPathName() const
{
	return m_stPathName;
}

bool CEterPack::Create(CEterFileDict& rkFileDict, const char * dbname, const char* pathName, bool bReadOnly)
{
	m_stPathName = pathName;

	strncpy_s(m_dbName, dbname, DBNAME_MAX_LEN);

	strncpy_s(m_indexFileName, dbname, MAX_PATH);
	strcat_s(m_indexFileName, ".eix");

	m_stDataFileName = dbname;
	m_stDataFileName += ".epk";

	m_bReadOnly = bReadOnly;

	// bReadOnly 모드가 아니고 데이터 베이스가 열린다면 생성 실패
	if (!CreateIndexFile())
		return false;

	if (!CreateDataFile())
		return false;

	__BuildIndex(rkFileDict);

	if (m_bReadOnly)
	{
		//m_bIsDataLoaded = true;
		//if (!m_file.Create(m_stDataFileName.c_str(), (const void**)&m_file_data, 0, 0))
		//	return false;
	}
	else
	{
		DecryptIndexFile();
	}

	return true;
}

bool CEterPack::DecryptIndexFile()
{
	if (!m_bEncrypted)
		return true;

	CFileBase file;

	if (!file.Create(m_indexFileName, CFileBase::FILEMODE_WRITE))
		return false;

	file.Write(&eterPack::c_IndexCC, sizeof(uint32_t));
	file.Write(&eterPack::c_Version, sizeof(uint32_t));
	file.Write(&m_indexCount, sizeof(int32_t));
	file.Write(m_indexData.data(), sizeof(TEterPackIndex) * m_indexCount);

	file.Close();

	m_bEncrypted = false;
	return true;
}

static DWORD s_adwEterPackKey[] =
{
	45129401,
	92367215,
	681285731,
	1710201,
};

static DWORD s_adwEterPackSecurityKey[] =
{
	78952482,
	527348324,
	1632942,
	486274726,
};

bool CEterPack::EncryptIndexFile()
{
	CMappedFile file;
	LPCVOID pvData;

	if (!file.Create(m_indexFileName, &pvData, 0, 0))
	{
		TraceError("EncryptIndex: Cannot open pack index file! %s", m_dbName);
		return false;
	}

	std::vector<uint8_t> pbData(file.Size());
	memcpy(pbData.data(), pvData, file.Size());

	CLZObject zObj;

	if (!CLZO::Instance().CompressEncryptedMemory(zObj, pbData.data(), file.Size(), s_adwEterPackKey))
	{
		TraceError("EncryptIndex: Cannot encrypt! %s", m_dbName);
		return false;
	}

	file.Destroy();

	while (!DeleteFile(m_indexFileName))
		continue;

	msl::file_ptr fPtr(m_indexFileName, "wb");

	if (!fPtr)
	{
		TraceError("EncryptIndex: Cannot open file for writing! %s", m_dbName);
		return false;
	}

	if (1 != fwrite(zObj.GetBuffer(), zObj.GetSize(), 1, fPtr.get()))
	{
		TraceError("Encryptindex: Cannot write to file! %s", m_indexFileName);
		return false;
	}

	m_bEncrypted = true;
	return true;
}

bool CEterPack::__BuildIndex(CEterFileDict& rkFileDict, bool bOverwrite)
{
	//uint32_t dwBeginTime = ELTimer_GetMSec();
	CMappedFile file;
	LPCVOID pvData;
	CLZObject zObj;

	if (!file.Create(m_indexFileName, &pvData, 0, 0))
	{
		TraceError("Cannot open pack index file! %s", m_dbName);
		return false;
	}

	if (file.Size() < eterPack::c_HeaderSize)
	{
		TraceError("Pack index file header error! %s", m_dbName);
		return false;
	}

	uint32_t fourcc = *(uint32_t *) pvData;

	uint8_t * pbData;
	uint32_t uiFileSize;

	if (fourcc == eterPack::c_IndexCC)
	{
		pbData = (uint8_t *) pvData;
		uiFileSize = file.Size();
	}
	else if (fourcc == CLZObject::ms_dwFourCC)
	{
		m_bEncrypted = true;

		if (!CLZO::Instance().Decompress(zObj, static_cast<const uint8_t *>(pvData), s_adwEterPackKey))
			return false;

		if (zObj.GetSize() < eterPack::c_HeaderSize)
			return false;

		pbData = zObj.GetBuffer();
		uiFileSize = zObj.GetSize();
	}
	else
	{
		TraceError("Pack index file fourcc error! %s", m_dbName);
		return false;
	}

	pbData += sizeof(uint32_t);

	uint32_t ver = *reinterpret_cast<uint32_t *>(pbData);
	pbData += sizeof(uint32_t);

	if (ver != eterPack::c_Version)
	{
		TraceError("Pack index file version error! %s", m_dbName);
		return false;
	}

	m_indexCount = *reinterpret_cast<int32_t *>(pbData);
	pbData += sizeof(int32_t);

	if (uiFileSize < eterPack::c_HeaderSize + sizeof(TEterPackIndex) * m_indexCount)
	{
		TraceError("Pack index file size error! %s, indexCount %d", m_dbName, m_indexCount);
		return false;
	}

	//Tracef("Loading Pack file %s elements: %d ... ", m_dbName, m_indexCount);

	m_indexData.resize(m_indexCount);
	memcpy(m_indexData.data(), pbData, sizeof(TEterPackIndex) * m_indexCount);

	TEterPackIndex * index = m_indexData.data();

	for (int32_t i = 0; i < m_indexCount; ++i, ++index)
	{
		if (!index->filename_crc)
		{
			PushFreeIndex(index);
		}
		else
		{
			if (index->real_data_size > index->data_size)
				m_FragmentSize += index->real_data_size - index->data_size;

			m_DataPositionMap.emplace(index->filename_crc, index);

			if (bOverwrite) // 서버 연동 패킹 파일은 나중에 들어오지만 최상위로 등록해야한다
				rkFileDict.UpdateItem(this, index);
			else
				rkFileDict.InsertItem(this, index);
		}
	}

	//Tracef("Done. (spent %dms)\n", ELTimer_GetMSec() - dwBeginTime);
	return true;
}
//
//void CEterPack::UpdateLastAccessTime()
//{
//	m_tLastAccessTime = time(nullptr);
//}
//
//void CEterPack::ClearDataMemoryMap()
//{
//	// m_file이 data file이다...
//	m_file.Destroy();
//	m_tLastAccessTime = 0;
//	m_bIsDataLoaded = false;
//}

bool CEterPack::Get(CMappedFile& out_file, const char * filename, LPCVOID * data)
{
	TEterPackIndex * index = FindIndex(filename);

	if (!index)
		return false;

	//UpdateLastAccessTime();
	//if (!m_bIsDataLoaded)
	//{
	//	if (!m_file.Create(m_stDataFileName.c_str(), (const void**)&m_file_data, 0, 0))
	//		return false;
	//
	//	m_bIsDataLoaded = true;
	//}

	// 기존에는 CEterPack에서 epk를 memory map에 올려놓고, 요청이 오면 그 부분을 링크해서 넘겨 줬었는데,
	// 이제는 요청이 오면, 필요한 부분만 memory map에 올리고, 요청이 끝나면 해제하게 함.
	out_file.Create(m_stDataFileName.c_str(), data, index->data_position, index->data_size);

	bool bIsSecurityCheckRequired = ( index->compressed_type == COMPRESSED_TYPE_SECURITY );


	if( bIsSecurityCheckRequired )
	{
		uint32_t dwCrc32 = GetCRC32(static_cast<const char *>(*data), index->data_size);

		if (index->data_crc != dwCrc32)
			return false;
	}

	if (COMPRESSED_TYPE_COMPRESS == index->compressed_type)
	{
		CLZObject * zObj = new CLZObject;

		if (!CLZO::Instance().Decompress(*zObj, static_cast<const uint8_t *>(*data)))
		{
			TraceError("Failed to decompress : %s", filename);
			delete zObj;
			return false;
		}

		out_file.BindLZObject(zObj);
		*data = zObj->GetBuffer();
	}
	else if (COMPRESSED_TYPE_SECURITY == index->compressed_type)
	{
		CLZObject * zObj = new CLZObject;

		if (!CLZO::Instance().Decompress(*zObj, static_cast<const uint8_t *>(*data), s_adwEterPackSecurityKey))
		{
			TraceError("Failed to encrypt : %s", filename);
			delete zObj;
			return false;
		}

		out_file.BindLZObject(zObj);
		*data = zObj->GetBuffer();
	}

	return true;
}

bool CEterPack::Get2(CMappedFile & out_file, const char * filename, TEterPackIndex * index, LPCVOID * data) const
{
	if (!index)
		return false;

	//UpdateLastAccessTime();
	//if (!m_bIsDataLoaded)
	//{
	//	if (!m_file.Create(m_stDataFileName.c_str(), (const void**)&m_file_data, 0, 0))
	//		return false;
	//
	//	m_bIsDataLoaded = true;
	//}
	out_file.Create(m_stDataFileName.c_str(), data, index->data_position, index->data_size);

	bool bIsSecurityCheckRequired = ( index->compressed_type == COMPRESSED_TYPE_SECURITY );


	if( bIsSecurityCheckRequired )
	{
		uint32_t dwCrc32 = GetCRC32(static_cast<const char *>(*data), index->data_size);

		if (index->data_crc != dwCrc32)
			return false;
	}

	if (COMPRESSED_TYPE_COMPRESS == index->compressed_type)
	{
		CLZObject * zObj = new CLZObject;

		if (!CLZO::Instance().Decompress(*zObj, static_cast<const uint8_t *>(*data)))
		{
			TraceError("Failed to decompress : %s", filename);
			delete zObj;
			return false;
		}

		out_file.BindLZObject(zObj);
		*data = zObj->GetBuffer();
	}
	else if (COMPRESSED_TYPE_SECURITY == index->compressed_type)
	{
		CLZObject * zObj = new CLZObject;

		if (!CLZO::Instance().Decompress(*zObj, static_cast<const uint8_t *>(*data), s_adwEterPackSecurityKey))
		{
			TraceError("Failed to encrypt : %s", filename);
			delete zObj;
			return false;
		}

		out_file.BindLZObject(zObj);
		*data = zObj->GetBuffer();
	}

	return true;
}


bool CEterPack::Delete(TEterPackIndex * pIndex)
{
	CFileBase fileIndex;

	if (!fileIndex.Create(m_indexFileName, CFileBase::FILEMODE_WRITE))
		return false;

	PushFreeIndex(pIndex);
	WriteIndex(fileIndex, pIndex);
	return true;
}

bool CEterPack::Delete(const char * filename)
{
	TEterPackIndex * pIndex = FindIndex(filename);

	if (!pIndex)
		return false;

	return Delete(pIndex);
}

bool CEterPack::Extract()
{
	CMappedFile dataMapFile;
	LPCVOID		data;

	if (!dataMapFile.Create(m_stDataFileName.c_str(), &data, 0, 0))
		return false;

	CLZObject zObj;

	for (auto & i : m_DataPositionMap)
	{
		TEterPackIndex * index = i.second;
		CFileBase writeFile;

		inlinePathCreate(index->filename);
		printf("%s\n", index->filename);

		writeFile.Create(index->filename, CFileBase::FILEMODE_WRITE);

		if (COMPRESSED_TYPE_COMPRESS == index->compressed_type)
		{
			if (!CLZO::Instance().Decompress(zObj, static_cast<const uint8_t *>(data) + index->data_position))
			{
				printf("cannot decompress");
				return false;
			}

			writeFile.Write(zObj.GetBuffer(), zObj.GetSize());
			zObj.Clear();
		}
		else if (COMPRESSED_TYPE_SECURITY == index->compressed_type)
		{
			if (!CLZO::Instance().Decompress(zObj, static_cast<const uint8_t *>(data) + index->data_position, s_adwEterPackSecurityKey))
			{
				printf("cannot decompress");
				return false;
			}

			writeFile.Write(zObj.GetBuffer(), zObj.GetSize());
			zObj.Clear();
		}

		else if (COMPRESSED_TYPE_NONE == index->compressed_type)
			writeFile.Write(static_cast<const char *>(data) + index->data_position, index->data_size);

		writeFile.Destroy();
	}
	return true;
}

bool CEterPack::GetNames(std::vector<std::string>* retNames)
{
	CMappedFile dataMapFile;
	LPCVOID		data;

	if (!dataMapFile.Create(m_stDataFileName.c_str(), &data, 0, 0))
		return false;

	CLZObject zObj;

	for (auto & i : m_DataPositionMap)
	{
		TEterPackIndex * index = i.second;

		inlinePathCreate(index->filename);

		retNames->emplace_back(index->filename);
	}
	return true;
}

bool CEterPack::Put(const char * filename, const char * sourceFilename, uint8_t packType, const std::string& strRelateMapName )
{
	CMappedFile mapFile;
	LPCVOID		data;

	if (sourceFilename)
	{
		if (!mapFile.Create(sourceFilename, &data, 0, 0))
			return false;
	}
	else if (!mapFile.Create(filename, &data, 0, 0))
		return false;

	uint8_t*  pMappedData    = (uint8_t*)data;
	int32_t	   iMappedDataSize = mapFile.Size();

	return Put(filename, pMappedData, iMappedDataSize, packType);
}

bool CEterPack::Put(const char * filename, LPCVOID data, int32_t len, uint8_t packType)
{
	if (m_bEncrypted)
	{
		TraceError("EterPack::Put : Cannot put to encrypted pack (filename: %s, DB: %s)", filename, m_dbName);
		return false;
	}

	CFileBase fileIndex;

	if (!fileIndex.Create(m_indexFileName, CFileBase::FILEMODE_WRITE))
		return false;

	CFileBase fileData;

	if (!fileData.Create(m_stDataFileName.c_str(), CFileBase::FILEMODE_WRITE))
		return false;

	TEterPackIndex * pIndex = FindIndex(filename);

	CLZObject zObj;
	std::string encryptStr;

	if (packType == COMPRESSED_TYPE_SECURITY ||
		packType == COMPRESSED_TYPE_COMPRESS)
	{
		if (packType == COMPRESSED_TYPE_SECURITY)
		{
			if (!CLZO::Instance().CompressEncryptedMemory(zObj, data, len, s_adwEterPackSecurityKey))
				return false;
		}
		else
		{
			if (!CLZO::Instance().CompressMemory(zObj, data, len))
				return false;
		}

		data = zObj.GetBuffer();
		len = zObj.GetSize();
	}

	uint32_t data_crc = GetCRC32(static_cast<const char *>(data), len);

	// 기존 데이터가 있으면..
	if (pIndex)
	{
		// 기존 data 크기가 넣을 데이터 크기를 수용할 수 있다면
		if (pIndex->real_data_size >= len)
		{
			++m_map_indexRefCount[pIndex->id];

			// 길이가 틀리거나, checksum이 틀릴 때만 저장 한다.
			if ( (pIndex->data_size != len) ||
				(pIndex->data_crc != data_crc) )
			{
				pIndex->data_size = len;
				pIndex->data_crc = data_crc;

				pIndex->compressed_type = packType;

				CMakePackLog::GetSingleton().Writef("Overwrite[type:%u] %s\n", pIndex->compressed_type, pIndex->filename);

				WriteIndex(fileIndex, pIndex);
				WriteData(fileData, pIndex, data);
			}

			return true;
		}

		// 기존 데이터 크기가 새로 들어갈 것 보다 적다면, 새로 인덱스를 할당해
		// 넣어야 한다. 원래 있던 인덱스는 비워 둔다.
		PushFreeIndex(pIndex);
		WriteIndex(fileIndex, pIndex);
	}

	// 새 데이터
	pIndex = NewIndex(fileIndex, filename, len);
	pIndex->data_size = len;

	pIndex->data_crc = data_crc;

	pIndex->data_position = GetNewDataPosition(fileData);
	pIndex->compressed_type = packType;

	WriteIndex(fileIndex, pIndex);
	WriteNewData(fileData, pIndex, data);

	++m_map_indexRefCount[pIndex->id];

	CMakePackLog::GetSingleton().Writef("Write[type:%u] %s\n", pIndex->compressed_type, pIndex->filename);

	return true;
}

int32_t CEterPack::GetFragmentSize() const
{
	return m_FragmentSize;
}

// Private methods
bool CEterPack::CreateIndexFile() const
{
	msl::file_ptr fPtr(m_indexFileName, "rb");
	if (fPtr) // close and exit if exists
		return true;

	if (m_bReadOnly)
		return false;

	//
	// 파일이 없으므로 새로 만든다.
	//
	fPtr.open(m_indexFileName, "wb");
	if (!fPtr)
		return false;

	fwrite(&eterPack::c_IndexCC, sizeof(uint32_t), 1, fPtr.get());
	fwrite(&eterPack::c_Version, sizeof(uint32_t), 1, fPtr.get());
	fwrite(&m_indexCount, sizeof(int32_t), 1, fPtr.get());
	return true;
}


void CEterPack::WriteIndex(CFileBase & file, TEterPackIndex * index) const
{
	file.Seek(sizeof(uint32_t) + sizeof(uint32_t));
	file.Write(&m_indexCount, sizeof(int32_t));
	file.Seek(eterPack::c_HeaderSize + (index->id * sizeof(TEterPackIndex)));

	if (!file.Write(index, sizeof(TEterPackIndex)))
		assert(!"WriteIndex: fwrite failed");
}

/*
 *	Free Block 이란 데이터에서 지워진 부분을 말한다.
 *	Free Block 들은 각각 FREE_INDEX_BLOCK_SIZE (32768) 단위로 나누어져
 *	리스트로 관리된다.
 *
 *	예를 들어 128k 의 데이터는
 *	128 * 1024 / FREE_INDEX_BLOCK_SIZE = 4 이므로
 *	최종 적으로는 m_FreeIndexList[4] 에 들어간다.
 *
 *	FREE_INDEX_BLOCK_SIZE 의 최대 값은 FREE_INDEX_MAX_SIZE(512) 이다.
 *	따라서 16MB 이상의 데이터는 무조건 배열의 512 위치에 들어간다.
 */
int32_t CEterPack::GetFreeBlockIndex(int32_t size)
{
	return std::min<int32_t>(FREE_INDEX_MAX_SIZE, size / FREE_INDEX_BLOCK_SIZE);
}

void CEterPack::PushFreeIndex(TEterPackIndex* index)
{
	if (index->filename_crc != 0)
	{
		auto i = m_DataPositionMap.find(index->filename_crc);

		if (i != m_DataPositionMap.end())
			m_DataPositionMap.erase(i);

		index->filename_crc = 0;
		memset(index->filename, 0, sizeof(index->filename));
	}

	int32_t blockidx = GetFreeBlockIndex(index->real_data_size);
	m_FreeIndexList[blockidx].emplace_back(index);
	m_FragmentSize += index->real_data_size;
}

int32_t CEterPack::GetNewIndexPosition(CFileBase & file)
{
	int32_t pos = (file.Size() - eterPack::c_HeaderSize) / sizeof(TEterPackIndex);
	++m_indexCount;
	return (pos);
}

TEterPackIndex* CEterPack::NewIndex(CFileBase& file, const char* filename, int32_t size)
{
	TEterPackIndex* index = nullptr;
	int32_t block_size = size + (DATA_BLOCK_SIZE - (size % DATA_BLOCK_SIZE));

	int32_t blockidx = GetFreeBlockIndex(block_size);

	for (auto i = m_FreeIndexList[blockidx].begin(); i != m_FreeIndexList[blockidx].end(); ++i)
	{
		 if ((*i)->real_data_size >= size)
		 {
			 index = *i;
			 m_FreeIndexList[blockidx].erase(i);

			 assert(index->filename_crc == 0);
			 break;
		 }
	}

	if (!index)
	{
		index = new TEterPackIndex;
		index->real_data_size = block_size;
		index->id = GetNewIndexPosition(file);
	}

	strncpy_s(index->filename, filename, FILENAME_MAX_LEN);
	index->filename[FILENAME_MAX_LEN] = '\0';
	inlineConvertPackFilename(index->filename);

	index->filename_crc	= GetCRC32(index->filename, strlen(index->filename));

	m_DataPositionMap.emplace(index->filename_crc, index);
	return index;
}

TEterPackIndex* CEterPack::FindIndex(const char * filename)
{
	static char tmpFilename[MAX_PATH + 1];
	strncpy_s(tmpFilename, filename, MAX_PATH);
	inlineConvertPackFilename(tmpFilename);

	uint32_t filename_crc = GetCRC32(tmpFilename, strlen(tmpFilename));

	auto i = m_DataPositionMap.find(filename_crc); 
	if (i != m_DataPositionMap.end())
		return (i->second);

	return nullptr;
}

bool CEterPack::IsExist(const char * filename)
{
	return FindIndex(filename) != nullptr;
}

bool CEterPack::CreateDataFile() const
{
	msl::file_ptr fPtr(m_stDataFileName.c_str(), "rb");
	if (fPtr) // close and exit if exists
		return true;

	if (m_bReadOnly)
		return false;

	fPtr.open(m_stDataFileName.c_str(), "wb");
	if (!fPtr)
		return false;

	return true;
}

int32_t CEterPack::GetNewDataPosition(CFileBase& file)
{
	return file.Size();
}

bool CEterPack::ReadData(CFileBase & file, TEterPackIndex* index, LPVOID data, int32_t maxsize)
{
	if (index->data_size > maxsize)
		return false;

	file.Seek(index->data_position);
	file.Read(data, index->data_size);
	return true;
}

bool CEterPack::WriteData(CFileBase & file, TEterPackIndex* index, LPCVOID data)
{
	file.Seek(index->data_position);

	if (!file.Write(data, index->data_size))
	{
		assert(!"WriteData: fwrite data failed");
		return false;
	}

	return true;
}

bool CEterPack::WriteNewData(CFileBase& file, TEterPackIndex* index, LPCVOID data)
{
	file.Seek(index->data_position);

	if (!file.Write(data, index->data_size))
	{
		assert(!"WriteData: fwrite data failed");
		return false;
	}

	const int32_t empty_size = index->real_data_size - index->data_size;

	if (empty_size < 0)
	{
		printf("SYSERR: WriteNewData(): CRITICAL ERROR: empty_size lower than 0!\n");
		exit(1);
	}

	if (empty_size == 0)
		return true;

	std::vector<char> empty_buf(empty_size);
	if (!file.Write(empty_buf.data(), empty_size))
	{
		assert(!"WriteData: fwrite empty data failed");
		return false;
	}
	return true;
}

TDataPositionMap & CEterPack::GetIndexMap()
{
	return m_DataPositionMap;
}

uint32_t CEterPack::DeleteUnreferencedData()
{
	TDataPositionMap::iterator i = m_DataPositionMap.begin();
	uint32_t dwCount = 0;

	while (i != m_DataPositionMap.end())
	{
		TEterPackIndex * pIndex = (i++)->second;

		if (0 == m_map_indexRefCount[pIndex->id])
		{
			printf("Unref File %s\n", pIndex->filename);
			Delete(pIndex);
			++dwCount;
		}
	}

	return dwCount;
}

const char * CEterPack::GetDBName() const
{
	return m_dbName;
}

#pragma optimize("", on)
/////////////////////////

void CEterFileDict::InsertItem(CEterPack* pkPack, TEterPackIndex* pkInfo)
{
	Item item;

	item.pkPack = pkPack;
	item.pkInfo = pkInfo;

	m_dict.emplace(pkInfo->filename_crc, item);
}

void CEterFileDict::UpdateItem(CEterPack* pkPack, TEterPackIndex* pkInfo)
{
	Item item;

	item.pkPack = pkPack;
	item.pkInfo = pkInfo;


	auto f = m_dict.find(pkInfo->filename_crc);
	if (f != m_dict.end())
	{
		if (strcmp(f->second.pkInfo->filename, item.pkInfo->filename) == 0)
			f->second = item;
		else
			TraceError("NAME_COLLISION: OLD: %s NEW: %s", f->second.pkInfo->filename, item.pkInfo->filename);
	}
	else
		m_dict.emplace(pkInfo->filename_crc, item);
}

CEterFileDict::Item* CEterFileDict::GetItem(uint32_t dwFileNameHash, const char * c_pszFileName)
{
	auto iter_pair = m_dict.equal_range(dwFileNameHash);
	auto iter = iter_pair.first;

	while (iter != iter_pair.second)
	{
		Item& item = iter->second;

		if (0 == strcmp(c_pszFileName, item.pkInfo->filename))
			return &item;

		++iter;
	}

	return nullptr;
}
