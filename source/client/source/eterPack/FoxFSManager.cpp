#include "StdAfx.h"
#include "EterPackManager.h"
#include "FileSystem.h"

#include <io.h>
#include <XORstr.h>

#include "../EterBase/Debug.h"
#include "../AntiCheatLib/CheatQueueManager.h"

#ifdef ENABLE_FOX_FS

const char* white_file_list[] = {"logininfo.xml", "mark\10_0.tga"};

bool isWhiteFile (std::string stFilename)
{
	for (auto whiteFile : white_file_list)
	{
		if (stricmp (stFilename.c_str(), whiteFile) == 0)
		{
			return true;
		}
	}

	return false;
}

const char* white_file_list_ext[] = {"xml", "tga", "png", "bmp", "mp3", "jpg"};

bool isWhiteFileExt (std::string stFilename)
{
	for (auto whiteFileExt : white_file_list_ext)
	{
		std::string ext = CFileNameHelper::GetExtension (stFilename);
		std::transform (ext.begin(), ext.end(), ext.begin(), ::tolower);
		std::string whiteExt = std::string (whiteFileExt);

		if (ext == whiteExt)
		{
			return true;
		}
	}

	return false;
}


bool CFileSystem::Get (CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData, const char* func, bool ignoreChecks)
{
	if (isExistInPack (c_szFileName, func))
	{
		return GetFromPack (rMappedFile, c_szFileName, pData);
	}


	if (isExist (c_szFileName, func, ignoreChecks))
	{
		return GetFromFile (rMappedFile, c_szFileName, pData);
	}


	return false;
}

void CFileSystem::OutputError(int32_t errorCode, bool showAsError, const std::string & stFilename, const char* func)
{
	std::wstring ws;
	ws.assign(stFilename.begin(), stFilename.end());
	OutputError(errorCode, showAsError, ws, func);
}
void CFileSystem::OutputError (int32_t errorCode, bool showAsError, const std::wstring & stFilename, const char* func)
{
	if (showAsError)
		TraceError("FileSystem :: Error for file %ls in function %s", stFilename.c_str(), func);
	else
		Tracenf ("FileSystem :: Error for file %ls in function %s", stFilename.c_str(), func);
	std::string error = "(null)";

	error = "Code: " + std::to_string(errorCode);
	/*
	switch (errorCode)
	{
		case FoxFS::ERROR_FILE_WAS_NOT_FOUND:
			error = ("FileSystem :: File not found");
			break;
		case FoxFS::ERROR_CORRUPTED_FILE:
			error = ("FileSystem :: File is corrupted");
			break;
		case FoxFS::ERROR_MISSING_KEY:
			error = ("FileSystem :: Missing key");
			break;
		case FoxFS::ERROR_MISSING_IV:
			error = ("FileSystem :: Missing IV");
			break;
		case FoxFS::ERROR_DECRYPTION_HAS_FAILED:
			error = ("FileSystem :: Could not decrypt");
			break;
		case FoxFS::ERROR_DECOMPRESSION_FAILED:
			error = ("FileSystem :: Could not decompress");
			break;
		case FoxFS::ERROR_ARCHIVE_NOT_FOUND:
			error = ("FileSystem :: Archive not found");
			break;
		case FoxFS::ERROR_ARCHIVE_NOT_READABLE:
			error = ("FileSystem :: Archive not readable");
			break;
		case FoxFS::ERROR_ARCHIVE_INVALID:
			error = ("FileSystem :: Archive invalid");
			break;
		case FoxFS::ERROR_ARCHIVE_ACCESS_DENIED:
			error = ("FileSystem :: Archive access denied");
			break;
		case FoxFS::ERROR_KEYSERVER_SOCKET:
			error = ("FileSystem :: Keyserver could not open socket");
			break;
		case FoxFS::ERROR_KEYSERVER_CONNECTION:
			error = ("FileSystem :: Keyserver could not connect");
			break;
		case FoxFS::ERROR_KEYSERVER_RESPONSE:
			error = ("FileSystem :: Keyserver invalid response");
			break;
		case FoxFS::ERROR_KEYSERVER_TIMEOUT:
			error = ("FileSystem :: Keyserver Timeout");
			break;

	}
	*/

	if (showAsError)
	{
		TraceError (error.c_str());
	}
	else
	{
		Tracenf (error.c_str());
	}

}

bool CFileSystem::GetFromPack (CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData)
{
	assert (c_szFileName);


	if (m_pFoxFS)
	{
		int32_t errorCodeSize = 0;

		if ((errorCodeSize = m_pFoxFS->exists(c_szFileName)) == 0)
		{
			uint32_t dwSize = m_pFoxFS->size(c_szFileName), dwReadSize = 0;
			uint8_t* pbData = new uint8_t[dwSize + 1];
			int32_t errorCode;

			if ((errorCode = m_pFoxFS->get(c_szFileName, pbData, dwSize, &dwReadSize)) == 0)
			{
				pbData[dwReadSize] = 0;
				*pData = pbData;
				rMappedFile.Link (dwReadSize, pbData);
				return true;
			}
			OutputError (errorCode, true, c_szFileName, __FUNCTION__);
			delete [] pbData;
		}
		else
		{
			OutputError (errorCodeSize, false, c_szFileName, __FUNCTION__);

		}
	}
	else
	{
		TraceError ("FileSystem :: Not initialized!");
	}

	return false;
}


bool CFileSystem::GetFromFile (CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData)
{
#ifndef _DEBUG
	if (!isExistInPack(c_szFileName, __FUNCTION__))
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

bool CFileSystem::isExistInPack (const char* c_szFileName, const char* func) const
{
	assert (c_szFileName);

	if (m_pFoxFS)
	{
		int32_t errorCodeSize = 0;
		if ((errorCodeSize = m_pFoxFS->exists(c_szFileName)) == 0)
		{
			return true;
		}

		//OutputError (errorCodeSize, false, std::string(c_szFileName), func);
	}
	else
	{
		TraceError ("FileSystem :: Not initialized!");
	}

	return false;
}

bool CFileSystem::isExist (const char* c_szFileName, const char* func, bool ignoreChecks) const
{

	if (isWhiteFile (c_szFileName) || isWhiteFileExt (c_szFileName) || ignoreChecks)
	{

		return isExistInPack (c_szFileName, func) || (_access (c_szFileName, 0) == 0);
	}
	return isExistInPack (c_szFileName, func);
}



bool CFileSystem::LoadArchive (const std::wstring & stName) const
{
	if (m_pFoxFS)
	{
		int32_t errorCode;
		if ((errorCode = m_pFoxFS->load(stName.c_str())) == 0)
			return true;

		OutputError (errorCode, true, stName, __FUNCTION__);
	}
	else
		TraceError ("FileSystem :: Not initialized!");

	return false;
}

bool CFileSystem::UnloadArchive (const std::wstring & stName) const
{
	if (m_pFoxFS)
	{
		int32_t errorCode;
		if ((errorCode = m_pFoxFS->unload(stName.c_str())) == 0)
			return true;

		OutputError(errorCode, true, stName, __FUNCTION__);
	}
	else
		TraceError("FileSystem :: Not initialized!");

	return false;
}



CFileSystem::CFileSystem() : m_pFoxFS (nullptr)
{

	m_pFoxFS = new FoxFS::FileSystem;
}

CFileSystem::~CFileSystem()
{
	if (m_pFoxFS)
	{
		delete m_pFoxFS;
		m_pFoxFS = nullptr;
	}
}

#endif

