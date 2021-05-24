#include "StdAfx.h"
#include "../eterPack/EterPackManager.h"
#include "../EterBase/CRC32.h"
#include "../EterBase/Timer.h"

#include "Resource.h"
#include "ResourceManager.h"

bool CResource::ms_bDeleteImmediately = false;

CResource::CResource(const char* c_szFileName) : me_state(STATE_EMPTY)
{
	SetFileName(c_szFileName);
}

CResource::~CResource()
{
}

void CResource::SetDeleteImmediately(bool isSet)
{
	ms_bDeleteImmediately = isSet;
}

void CResource::OnConstruct()
{
	Load();
}

void CResource::OnSelfDestruct()
{
	if (ms_bDeleteImmediately)
		Clear();
	else
		CResourceManager::Instance().ReserveDeletingResource(this);
}

void CResource::Load()
{
	if (me_state != STATE_EMPTY)
		return;

	const char * c_szFileName = GetFileName();

	DWORD		dwStart = ELTimer_GetMSec();
	CMappedFile	file;
	LPCVOID		fileData;
#ifdef ENABLE_FOX_FS
	if (CFileSystem::Instance().Get(file, c_szFileName, &fileData, __FUNCTION__))
#else
	if (CEterPackManager::Instance().Get(file, c_szFileName, &fileData))
#endif
	{
		m_dwLoadCostMiliiSecond = ELTimer_GetMSec() - dwStart;
		if (OnLoad(file.Size(), fileData))
		{
			me_state = STATE_EXIST;
		}
		else
		{
			Tracef("CResource::Load Error %s\n", c_szFileName);
			me_state = STATE_ERROR;
			return;
		}
	}
	else
	{
		if (OnLoad(0, NULL))
			me_state = STATE_EXIST;
		else
		{
			Tracef("CResource::Load file not exist %s\n", c_szFileName);
			me_state = STATE_ERROR;
		}
	}
}

void CResource::Reload()
{
	Clear();
	Tracef("CResource::Reload %s\n", GetFileName());

	CMappedFile	file;
	LPCVOID		fileData;

#ifdef ENABLE_FOX_FS
	if (CFileSystem::Instance().Get(file, GetFileName(), &fileData, __FUNCTION__))
#else
	if (CEterPackManager::Instance().Get(file, GetFileName(), &fileData))
#endif
	{
		if (OnLoad(file.Size(), fileData))
		{
			me_state = STATE_EXIST;
		}
		else
		{
			me_state = STATE_ERROR;
			return;
		}
	}
	else
	{
		if (OnLoad(0, NULL))
			me_state = STATE_EXIST;
		else
		{
			me_state = STATE_ERROR;
		}
	}
}

CResource::TType CResource::StringToType(const char* c_szType)
{
	return GetCRC32(c_szType, strlen(c_szType));
}

int CResource::ConvertPathName(const char * c_szPathName, char * pszRetPathName, int retLen)
{
	const char * pc;
	int len = 0;

	for (pc = c_szPathName; *pc && len < retLen; ++pc, ++len)
	{
		if (*pc == '/')
			*(pszRetPathName++) = '\\';
		else
			*(pszRetPathName++) = (char) korean_tolower(*pc);
	}

	*pszRetPathName = '\0';
	return len;
}

void CResource::SetFileName(const char* c_szFileName)
{
	m_stFileName=c_szFileName;
}

void CResource::Clear()
{
	OnClear();
	me_state = STATE_EMPTY;
}

bool CResource::IsType(TType type)
{
	return OnIsType(type);
}

CResource::TType CResource::Type()
{
	static TType s_type = StringToType("CResource");
	return s_type;
}

bool CResource::OnIsType(TType type)
{
	if (CResource::Type() == type)
		return true;

	return false;
}

bool CResource::IsData() const
{
	return me_state != STATE_EMPTY;
}

bool CResource::IsEmpty() const
{
	return OnIsEmpty();
}

bool CResource::CreateDeviceObjects()
{
	return true;
}

void CResource::DestroyDeviceObjects()
{
}
