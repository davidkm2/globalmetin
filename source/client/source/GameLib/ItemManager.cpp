#include "StdAfx.h"
#include "../eterPack/EterPackManager.h"
#include "../EterLib/ResourceManager.h"

#include "ItemManager.h"

static DWORD s_adwItemProtoKey[4] =
{
	173217,
	72619434,
	408587239,
	27973291
};

BOOL CItemManager::SelectItemData(DWORD dwIndex)
{
	TItemMap::iterator f = m_ItemMap.find(dwIndex);

	if (m_ItemMap.end() == f)
	{
		int n = m_vec_ItemRange.size();
		for (int i = 0; i < n; i++)
		{
			CItemData * p = m_vec_ItemRange[i];
			const CItemData::TItemTable * pTable = p->GetTable();
			if ((pTable->dwVnum < dwIndex) &&
				dwIndex < (pTable->dwVnum + pTable->dwVnumRange))
			{
				m_pSelectedItemData = p;
				return TRUE;
			}
		}
		Tracef(" CItemManager::SelectItemData - FIND ERROR [%d]\n", dwIndex);
		return FALSE;
	}

	m_pSelectedItemData = f->second;

	return TRUE;
}

CItemData * CItemManager::GetSelectedItemDataPointer()
{
	return m_pSelectedItemData;
}

BOOL CItemManager::GetItemDataPointer(DWORD dwItemID, CItemData ** ppItemData)
{
	if (0 == dwItemID)
		return FALSE;

	TItemMap::iterator f = m_ItemMap.find(dwItemID);

	if (m_ItemMap.end() == f)
	{
		int n = m_vec_ItemRange.size();
		for (int i = 0; i < n; i++)
		{
			CItemData * p = m_vec_ItemRange[i];
			const CItemData::TItemTable * pTable = p->GetTable();
			if ((pTable->dwVnum < dwItemID) &&
				dwItemID < (pTable->dwVnum + pTable->dwVnumRange))
			{
				*ppItemData = p;
				return TRUE;
			}
		}
		Tracef(" CItemManager::GetItemDataPointer - FIND ERROR [%d]\n", dwItemID);
		return FALSE;
	}

	*ppItemData = f->second;

	return TRUE;
}

CItemData * CItemManager::MakeItemData(DWORD dwIndex)
{
	TItemMap::iterator f = m_ItemMap.find(dwIndex);

	if (m_ItemMap.end() == f)
	{
		CItemData * pItemData = CItemData::New();

		m_ItemMap.insert(TItemMap::value_type(dwIndex, pItemData));

		return pItemData;
	}

	return f->second;
}
bool CItemManager::LoadItemList(const char * c_szFileName)
{
	CMappedFile File;
	LPCVOID pData;

#ifdef ENABLE_FOX_FS
	if (!CFileSystem::Instance().Get(File, c_szFileName, &pData, __FUNCTION__))
#else
	if (!CEterPackManager::Instance().Get(File, c_szFileName, &pData))
#endif
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(File.Size(), pData);

	CTokenVector TokenVector;
    for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			continue;

		if (!(TokenVector.size() == 3 || TokenVector.size() == 4))
		{
			TraceError(" CItemManager::LoadItemList(%s) - StrangeLine in %d\n", c_szFileName, i);
			continue;
		}

		const std::string & c_rstrID = TokenVector[0];

		const std::string & c_rstrIcon = TokenVector[2];

		DWORD dwItemVNum=atoi(c_rstrID.c_str());

		CItemData * pItemData = MakeItemData(dwItemVNum);

		extern BOOL USE_VIETNAM_CONVERT_WEAPON_VNUM;
		if (USE_VIETNAM_CONVERT_WEAPON_VNUM)
		{
			extern DWORD Vietnam_ConvertWeaponVnum(DWORD vnum);
			DWORD dwMildItemVnum = Vietnam_ConvertWeaponVnum(dwItemVNum);
			if (dwMildItemVnum == dwItemVNum)
			{
				if (4 == TokenVector.size())
				{
					const std::string & c_rstrModelFileName = TokenVector[3];
					pItemData->SetDefaultItemData(c_rstrIcon.c_str(), c_rstrModelFileName.c_str());
				}
				else
					pItemData->SetDefaultItemData(c_rstrIcon.c_str());
			}
			else
			{
				DWORD dwMildBaseVnum = dwMildItemVnum / 10 * 10;
				char szMildIconPath[MAX_PATH];
				sprintf(szMildIconPath, "icon/item/%.5d.tga", dwMildBaseVnum);
				if (4 == TokenVector.size())
				{
					char szMildModelPath[MAX_PATH];
					sprintf(szMildModelPath, "d:/ymir work/item/weapon/%.5d.gr2", dwMildBaseVnum);
					pItemData->SetDefaultItemData(szMildIconPath, szMildModelPath);
				}
				else
					pItemData->SetDefaultItemData(szMildIconPath);
			}
		}
		else
		{
			if (4 == TokenVector.size())
			{
				const std::string & c_rstrModelFileName = TokenVector[3];
				pItemData->SetDefaultItemData(c_rstrIcon.c_str(), c_rstrModelFileName.c_str());
			}
			else
				pItemData->SetDefaultItemData(c_rstrIcon.c_str());
		}
	}

	return true;
}

const std::string& __SnapString(const std::string& c_rstSrc, std::string& rstTemp)
{
	UINT uSrcLen=c_rstSrc.length();
	if (uSrcLen<2)
		return c_rstSrc;

	if (c_rstSrc[0]!='"')
		return c_rstSrc;

	UINT uLeftCut=1;

	UINT uRightCut=uSrcLen;
	if (c_rstSrc[uSrcLen-1]=='"')
		uRightCut=uSrcLen-1;

	rstTemp=c_rstSrc.substr(uLeftCut, uRightCut-uLeftCut);
	return rstTemp;
}

bool CItemManager::LoadItemDesc(const char* c_szFileName)
{
	const VOID* pvData;
	CMappedFile kFile;
#ifdef ENABLE_FOX_FS
	if (!CFileSystem::Instance().Get(kFile, c_szFileName, &pvData, __FUNCTION__))
#else
	if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
#endif
	{
		Tracenf("CItemManager::LoadItemDesc(c_szFileName=%s) - Load Error", c_szFileName);
		return false;
	}

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.Size(), pvData);

	std::string stTemp;

	CTokenVector kTokenVector;
	for (DWORD i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		if (!kTextFileLoader.SplitLineByTab(i, &kTokenVector))
			continue;

		while (kTokenVector.size()<ITEMDESC_COL_NUM)
			kTokenVector.push_back("");
		DWORD dwVnum=atoi(kTokenVector[ITEMDESC_COL_VNUM].c_str());
		const std::string& c_rstDesc=kTokenVector[ITEMDESC_COL_DESC];
		const std::string& c_rstSumm=kTokenVector[ITEMDESC_COL_SUMM];
		TItemMap::iterator f = m_ItemMap.find(dwVnum);
		if (m_ItemMap.end() == f)
			continue;

		CItemData* pkItemDataFind = f->second;

		pkItemDataFind->SetDescription(__SnapString(c_rstDesc, stTemp));
		pkItemDataFind->SetSummary(__SnapString(c_rstSumm, stTemp));
	}
	return true;
}

DWORD GetHashCode( const char* pString )
{
	   unsigned long i,len;
	   unsigned long ch;
	   unsigned long result;

	   len     = strlen( pString );
	   result = 5381;
	   for( i=0; i<len; i++ )
	   {
	   	   ch = (unsigned long)pString[i];
	   	   result = ((result<< 5) + result) + ch; 
	   }

	   return result;
}

bool CItemManager::LoadItemTable(const char* c_szFileName)
{
	CMappedFile file;
	LPCVOID pvData;

#ifdef ENABLE_FOX_FS
	if (!CFileSystem::Instance().Get(file, c_szFileName, &pvData, __FUNCTION__))
#else
	if (!CEterPackManager::Instance().Get(file, c_szFileName, &pvData))
#endif
		return false;

	DWORD dwFourCC, dwElements, dwDataSize;
	DWORD dwVersion=0;
	DWORD dwStride=0;

	file.Read(&dwFourCC, sizeof(DWORD));

	if (dwFourCC == MAKEFOURCC('M', 'I', 'P', 'X'))
	{
		file.Read(&dwVersion, sizeof(DWORD));
		file.Read(&dwStride, sizeof(DWORD));

		if (dwVersion != 1)
		{
			TraceError("CPythonItem::LoadItemTable: invalid item_proto[%s] VERSION[%d]", c_szFileName, dwVersion);
			return false;
		}

#ifdef ENABLE_PROTOSTRUCT_AUTODETECT
		if (!CItemData::TItemTableAll::IsValidStruct(dwStride))
#else
		if (dwStride != sizeof(CItemData::TItemTable))
#endif
		{
			TraceError("CPythonItem::LoadItemTable: invalid item_proto[%s] STRIDE[%d] != sizeof(SItemTable)",
				c_szFileName, dwStride, sizeof(CItemData::TItemTable));
			return false;
		}
	}
	else if (dwFourCC != MAKEFOURCC('M', 'I', 'P', 'T'))
	{
		TraceError("CPythonItem::LoadItemTable: invalid item proto type %s", c_szFileName);
		return false;
	}

	file.Read(&dwElements, sizeof(DWORD));
	file.Read(&dwDataSize, sizeof(DWORD));

	BYTE * pbData = new BYTE[dwDataSize];
	file.Read(pbData, dwDataSize);
	CLZObject zObj;

	if (!CLZO::Instance().Decompress(zObj, pbData, s_adwItemProtoKey))
	{
		delete [] pbData;
		return false;
	}
	char szName[64+1];
	std::map<DWORD,DWORD> itemNameMap;

	for (DWORD i = 0; i < dwElements; ++i)
	{
#ifdef ENABLE_PROTOSTRUCT_AUTODETECT
		CItemData::TItemTable t = {0};
		CItemData::TItemTableAll::Process(zObj.GetBuffer(), dwStride, i, t);
#else
		CItemData::TItemTable & t = *((CItemData::TItemTable *) zObj.GetBuffer() + i);
#endif
		CItemData::TItemTable * table = &t;

		CItemData * pItemData;
		DWORD dwVnum = table->dwVnum;

		TItemMap::iterator f = m_ItemMap.find(dwVnum);
		if (m_ItemMap.end() == f)
		{
			_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", dwVnum);

#ifdef ENABLE_FOX_FS
			if (CResourceManager::Instance().IsFileExist(szName, __FUNCTION__) == false)
#else
			if (CResourceManager::Instance().IsFileExist(szName) == false)
#endif
			{
				std::map<DWORD, DWORD>::iterator itVnum = itemNameMap.find(GetHashCode(table->szName));

				if (itVnum != itemNameMap.end())
					_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", itVnum->second);
				else
					_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", dwVnum-dwVnum % 10);

#ifdef ENABLE_FOX_FS
				if (CResourceManager::Instance().IsFileExist(szName, __FUNCTION__) == false)
#else
				if (CResourceManager::Instance().IsFileExist(szName) == false)
#endif
				{
					#ifdef __TEST__
					TraceError("%16s(#%-5d) cannot find icon file. setting to default.", table->szName, dwVnum);
					#endif
					const DWORD EmptyBowl = 27995;
					_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", EmptyBowl);
				}
			}

			pItemData = CItemData::New();

			pItemData->SetDefaultItemData(szName);
			m_ItemMap.insert(TItemMap::value_type(dwVnum, pItemData));
		}
		else
		{
			pItemData = f->second;
		}
		if (itemNameMap.find(GetHashCode(table->szName)) == itemNameMap.end())
			itemNameMap.insert(std::map<DWORD,DWORD>::value_type(GetHashCode(table->szName),table->dwVnum));
		pItemData->SetItemTableData(table);
		if (0 != table->dwVnumRange)
		{
			m_vec_ItemRange.push_back(pItemData);
		}
	}

	delete [] pbData;
	return true;
}

#ifdef ENABLE_ACCE_SYSTEM
bool CItemManager::LoadItemScale(const char * c_szFileName)
{
	const VOID* pvData;
	CMappedFile kFile;
#ifdef ENABLE_FOX_FS
	if (!CFileSystem::Instance().Get(kFile, c_szFileName, &pvData, __FUNCTION__))
#else
	if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
#endif
		return false;
	
	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.Size(), pvData);
	
	CTokenVector kTokenVector;
	for (DWORD i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		if (!kTextFileLoader.SplitLineByTab(i, &kTokenVector))
			continue;
		
		if (kTokenVector.size() < ITEMSCALE_NUM - 3)
		{
			TraceError("LoadItemScale: invalid line %d (%s).", i, c_szFileName);
			continue;
		}
		
		const std::string & strJob = kTokenVector[ITEMSCALE_JOB];
		const std::string & strSex = kTokenVector[ITEMSCALE_SEX];
		const std::string & strScaleX = kTokenVector[ITEMSCALE_SCALE_X];
		const std::string & strScaleY = kTokenVector[ITEMSCALE_SCALE_Y];
		const std::string & strScaleZ = kTokenVector[ITEMSCALE_SCALE_Z];
		const std::string & strPositionX = kTokenVector[ITEMSCALE_POSITION_X];
		const std::string & strPositionY = kTokenVector[ITEMSCALE_POSITION_Y];
		const std::string & strPositionZ = kTokenVector[ITEMSCALE_POSITION_Z];
		
		for (int j = 0; j < 5; ++j)
		{
			CItemData * pItemData = MakeItemData(atoi(kTokenVector[ITEMSCALE_VNUM].c_str()) + j);
			pItemData->SetItemScale(strJob, strSex, strScaleX, strScaleY, strScaleZ, strPositionX, strPositionY, strPositionZ);
		}
	}
	
	return true;
}
#endif

void CItemManager::Destroy()
{
	TItemMap::iterator i;
	for (i=m_ItemMap.begin(); i!=m_ItemMap.end(); ++i)
		CItemData::Delete(i->second);

	m_ItemMap.clear();
}

CItemManager::CItemManager() : m_pSelectedItemData(NULL)
{
}
CItemManager::~CItemManager()
{
	Destroy();
}
