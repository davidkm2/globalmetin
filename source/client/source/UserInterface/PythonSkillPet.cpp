#include "StdAfx.h"
#include "PythonSkillPet.h"
#include "../EterBase/Poly/Poly.h"
#include "../EterPack/EterPackManager.h"
#include "InstanceBase.h"
#include "PythonPlayer.h"

bool CPythonSkillPet::RegisterSkillPet(const char * c_szFileName)
{
	const VOID* pvData;
	CMappedFile kFile;
#ifdef ENABLE_FOX_FS
	if (!CFileSystem::Instance().Get(kFile, c_szFileName, &pvData, __FUNCTION__))
#else
	if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
#endif
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(kFile.Size(), pvData);
	
	CTokenVector TokenVector;
	for (DWORD i = 0; i < textFileLoader.GetLineCount()-1; ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			return false;

		if (DESCPET_TOKEN_TYPE_MAX_NUM > TokenVector.size())
			return false;

		DWORD iSkillIndex = atoi(TokenVector[DESCPET_TOKEN_TYPE_VNUM].c_str());
		if (iSkillIndex == 0)
			continue;

		m_SkillDataPetMap.insert(make_pair(iSkillIndex, TSkillDataPet()));

		TSkillDataPet & rSkillData = m_SkillDataPetMap[iSkillIndex];

		rSkillData.dwSkillIndex = iSkillIndex;		

		rSkillData.strName = TokenVector[DESCPET_TOKEN_TYPE_NAME];

		char szIconFileNameHeader[64+1];
		_snprintf(szIconFileNameHeader, sizeof(szIconFileNameHeader), "%sskill/pet/%s.sub", g_strImagePath.c_str(), TokenVector[DESCPET_TOKEN_TYPE_ICON_NAME].c_str());

		rSkillData.strIconFileName = szIconFileNameHeader;
		std::map<std::string, DWORD>::iterator itor = m_SkillPetTypeIndexMap.begin();
		for(; itor != m_SkillPetTypeIndexMap.end(); ++itor)
			if (TokenVector[DESCPET_TOKEN_TYPE].compare(itor->first) == 0)
					rSkillData.byType = itor->second;

		rSkillData.strDescription = TokenVector[DESCPET_TOKEN_TYPE_DESCRIPTION];
		rSkillData.dwskilldelay = atoi(TokenVector[DESCPET_TOKEN_TYPE_DELAY].c_str());

		rSkillData.pImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(szIconFileNameHeader);			
	}

	return true;
}

void CPythonSkillPet::__RegisterNormalIconImage(TSkillDataPet & rData, const char * c_szHeader, const char * c_szImageName)
{
	std::string strFileName = "";
	strFileName += c_szHeader;
	strFileName += c_szImageName;
	strFileName += ".sub";
	rData.pImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(strFileName.c_str());
}

void CPythonSkillPet::Destroy()
{
	m_SkillDataPetMap.clear();	
}

CPythonSkillPet::SSkillDataPet::SSkillDataPet()
{
	dwSkillIndex = 0;
	strName = "";
	strIconFileName = "";
	byType = 0;			
	strDescription = "";
	dwskilldelay = 0;
	pImage = NULL;	
	
}

CPythonSkillPet::CPythonSkillPet()
{
	petslot[0] = 0;
	petslot[1] = 0;
	petslot[2] = 0;

	m_SkillPetTypeIndexMap.insert(std::map<std::string, DWORD>::value_type("EMPTY", SKILLPET_TYPE_NONE));
	m_SkillPetTypeIndexMap.insert(std::map<std::string, DWORD>::value_type("PASSIVE", SKILLPET_TYPE_PASSIVE));
	m_SkillPetTypeIndexMap.insert(std::map<std::string, DWORD>::value_type("AUTO", SKILLPET_TYPE_AUTO));

}

CPythonSkillPet::~CPythonSkillPet()
{
}

void CPythonSkillPet:: SetSkillbySlot(int slot, int skillIndex)
{
	petslot[slot] = skillIndex;
}

BOOL CPythonSkillPet::GetSkillData(DWORD dwSkillIndex, TSkillDataPet ** ppSkillData)
{
	TSkillDataPetMap::iterator it = m_SkillDataPetMap.find(dwSkillIndex);

	if (m_SkillDataPetMap.end() == it)
		return FALSE;

	*ppSkillData = &(it->second);
	return TRUE;
}

BOOL CPythonSkillPet::GetSkillIndex(int slot, int* skillIndex) 
{
	*skillIndex = petslot[slot];
	return TRUE;
}

PyObject * petskillSetSkillSlot(PyObject * poSelf, PyObject * poArgs)
{
	int slot;
	int skillIndex;

	if (!PyTuple_GetInteger(poArgs, 0, &slot))
		return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 1, &skillIndex))
		return Py_BadArgument();
	CPythonSkillPet::Instance().SetSkillbySlot(slot, skillIndex);
	return Py_BuildValue("i", 0);
}

PyObject * petskillGetIconImage(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkillPet::SSkillDataPet * c_pSkillData;
	if (!CPythonSkillPet::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", c_pSkillData->pImage);
}

PyObject * petskillGetSkillbySlot(PyObject * poSelf, PyObject * poArgs)
{
	int slot;
	int skillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &slot))
		return Py_BadArgument();

	if (slot > 2)
		return Py_BadArgument();

	if (!CPythonSkillPet::Instance().GetSkillIndex(slot, &skillIndex))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", skillIndex);

}

PyObject * petskillGetEmptySkill(PyObject * poSelf, PyObject * poArgs)
{
	CGraphicImage * noskillbtn;
	char szIconFileNameHeader[64 + 1];
	_snprintf(szIconFileNameHeader, sizeof(szIconFileNameHeader), "d:/ymir work/ui/pet/skill_button/skill_enable_button.sub");
	noskillbtn = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(szIconFileNameHeader);
	return Py_BuildValue("i", noskillbtn);
}

void initskillpet()
{
	static PyMethodDef s_methods[] =
	{
		{ "SetSkillSlot",					petskillSetSkillSlot,						METH_VARARGS },		
		{ "GetIconImage",					petskillGetIconImage,						METH_VARARGS },
		{ "GetSkillbySlot",					petskillGetSkillbySlot ,					METH_VARARGS },
		{ "GetEmptySkill",					petskillGetEmptySkill,						METH_VARARGS },

		{ NULL,										NULL,										NULL },
	};

	PyObject * poModule = Py_InitModule("petskill", s_methods);
}

