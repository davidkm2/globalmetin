#include "stdafx.h"
#include <random>
#include "PythonDynamicModule.h"
#include "../eterPack/EterPackManager.h"
#include "../AntiCheatLib/PythonStackCheck.h"
#include <XORstr.h>

auto charset()
{
	return std::vector<char>(
	{ '0','1','2','3','4',
		'5','6','7','8','9',
		'A','B','C','D','E','F',
		'G','H','I','J','K',
		'L','M','N','O','P',
		'Q','R','S','T','U',
		'V','W','X','Y','Z',
		'a','b','c','d','e','f',
		'g','h','i','j','k',
		'l','m','n','o','p',
		'q','r','s','t','u',
		'v','w','x','y','z'
	});
};

CPythonDynamicModule::CPythonDynamicModule()
{
	Initialize();
}

void CPythonDynamicModule::Initialize()
{
	for (uint8_t i = 0; i < PYTHON_MODULE_MAX; ++i)
	{
		m_dynamicModuleNameArray[i] = GenerateRandomString();
#ifdef _DEBUG
		Tracenf("Created name: '%s' for module %d", m_dynamicModuleNameArray[i].c_str(), i);
#endif
	}
}

std::string CPythonDynamicModule::GenerateRandomString()
{
	const auto ch_set = charset();

	std::default_random_engine rng(std::random_device{}());
	std::uniform_int_distribution<> dist(0, ch_set.size() - 1);

	auto randchar = [ch_set, &dist, &rng]() {return ch_set[dist(rng)]; };

	std::uniform_int_distribution<> distLen(10, 20);

	auto length = distLen(rng);
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);

	return str;
}

std::string CPythonDynamicModule::GetModule(uint8_t nModuleID) const
{
#ifndef PYTHON_DYNAMIC_MODULE_NAME
	switch (nModuleID)
	{
		case PYTHON_APPLICATION:
			return "app";
		case PYTHON_CHARACTER:
			return "chr";
		case PYTHON_CHARACTER_MGR:
			return "chrmgr";
		case PYTHON_NETWORK:
			return "net";
		case PYTHON_PACK:
			return "pack";
		case PYTHON_PLAYER:
			return "player";
		default:
			TraceError("Unknown module request: %u", nModuleID);
			return "";
	}
#else
	return m_dynamicModuleNameArray[nModuleID];
#endif
}

PyObject * pythonImportManager(PyObject * poSelf, PyObject * poArgs)
{
	char __s[] = { 's', 0x0 }; // s
	std::string stReferFile = PY_REF_FILE;
	std::string stReferFunc = PY_REF_FUNC;

	char* szOldName;
	if (!PyTuple_GetString(poArgs, 0, &szOldName))
		return Py_BuildException();

#ifndef PYTHON_DYNAMIC_MODULE_NAME
	return Py_BuildValue(__s, szOldName);
#endif

	if (!stReferFile.empty())
	{
#ifdef ENABLE_FOX_FS
		auto bReferInPack = CFileSystem::Instance().isExistInPack(stReferFile.c_str(), __FUNCTION__);
#else
		auto bReferInPack = CEterPackManager::Instance().isExistInPack(stReferFile.c_str());
#endif

		char __locale[] = { 'l', 'o', 'c', 'a', 'l', 'e', '/', 0x0 }; // locale/
		char __uiscript[] = { 'u', 'i', 's', 'c', 'r', 'i', 'p', 't', '/', 0x0 }; // uiscript/

		if (!bReferInPack &&
			!strstr(stReferFile.c_str(), __locale) && !strstr(stReferFile.c_str(), __uiscript))
		{
			TraceError(XOR("Unknown module import reference from '%s':'%s'"), stReferFile.c_str(), stReferFunc.c_str());
			return Py_BuildNone();
		}
	}

	char __app[] = { 'a', 'p', 'p', 0x0 }; // app
	char __chr[] = { 'c', 'h', 'r', 0x0 }; // chr
	char __chrmgr[] = { 'c', 'h', 'r', 'm', 'g', 'r', 0x0 }; // chrmgr
	char __net[] = { 'n', 'e', 't', 0x0 }; // net
	char __pack[] = { 'p', 'a', 'c', 'k', 0x0 }; // pack
	char __player[] = { 'p', 'l', 'a', 'y', 'e', 'r', 0x0 }; // player

	if (!strcmp(szOldName, __app))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(PYTHON_APPLICATION).c_str());

	else if (!strcmp(szOldName, __chr))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(PYTHON_CHARACTER).c_str());

	else if (!strcmp(szOldName, __chrmgr))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(PYTHON_CHARACTER_MGR).c_str());

	else if (!strcmp(szOldName, __net))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(PYTHON_NETWORK).c_str());

	else if (!strcmp(szOldName, __pack))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(PYTHON_PLAYER).c_str());

	else if (!strcmp(szOldName, __player))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(PYTHON_PLAYER).c_str());

	else
		TraceError(XOR("Error! Unknown module name('%s') request from '%s':'%s'"), szOldName, stReferFile.c_str(), stReferFunc.c_str());

	return Py_BuildNone();
}

void initDynamicModuleMgr()
{
	char szModuleName[] = { 'i', 'm', 'p', 'M', 'g', 'r', 0x0 };
	char szGetMethod[]  = { 'g', 'e', 't', 'M', 'o', 'd', 'u', 'l', 'e', 0x0 };

	static PyMethodDef s_methods[] =
	{
		{ szGetMethod,					pythonImportManager,				METH_VARARGS},
		{ NULL,							NULL,								NULL },
	};
	Py_InitModule(szModuleName, s_methods);
}
