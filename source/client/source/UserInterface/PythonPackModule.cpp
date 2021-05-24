#include "StdAfx.h"
#include "../eterPack/EterPackManager.h"
#include "../AntiCheatLib/PythonStackCheck.h"
#include "PythonDynamicModule.h"

PyObject * packExist(PyObject * poSelf, PyObject * poArgs)
{
	bool bIsSystem = false;
	if (!PyTuple_GetBoolean(poArgs, 0, &bIsSystem) || !bIsSystem)
		return Py_BuildValue("i", 0);

	char * strFileName;
	if (!PyTuple_GetString(poArgs, 1, &strFileName))
		return Py_BuildException();

#ifdef ENABLE_FOX_FS
	bool exist = CFileSystem::Instance().isExist(strFileName, __FUNCTION__, false);
#else
	bool exist = CEterPackManager::Instance().isExist(strFileName);
#endif

	return Py_BuildValue("i", exist ? 1 : 0);
}

PyObject * packExistInPack(PyObject * poSelf, PyObject * poArgs)
{
	char * strFileName;
	if (!PyTuple_GetString(poArgs, 0, &strFileName))
		return Py_BuildException();

#ifdef ENABLE_FOX_FS
	return Py_BuildValue("i", CFileSystem::Instance().isExistInPack(strFileName, __FUNCTION__) ? 1 : 0);
#else
	return Py_BuildValue("i", CEterPackManager::Instance().isExistInPack(strFileName) ? 1 : 0);
#endif
}

PyObject * packGet(PyObject * poSelf, PyObject * poArgs)
{
	std::string stReferFile = PY_REF_FILE;
	char szFormat[] = { 's', '#', 0x0 };

	bool bIsSystem = false;
	if (!PyTuple_GetBoolean(poArgs, 0, &bIsSystem) || !bIsSystem)
		return Py_BuildException();

	char * strFileName;
	if (!PyTuple_GetString(poArgs, 1, &strFileName))
		return Py_BuildException();

#ifdef ENABLE_FOX_FS
	if (CFileSystem::Instance().isExistInPack(stReferFile.c_str(), __FUNCTION__))
#else
	if (CEterPackManager::Instance().isExistInPack(stReferFile.c_str()))
#endif
	{
#ifdef ENABLE_FOX_FS
		if (CFileSystem::Instance().isExist(strFileName, __FUNCTION__, false))
#else
		if (CEterPackManager::Instance().isExist(strFileName))
#endif
		{
			CMappedFile file;
			const void * pData = NULL;

#ifdef ENABLE_FOX_FS
			if (CFileSystem::Instance().Get(file, strFileName, &pData, __FUNCTION__))
#else
			if (CEterPackManager::Instance().Get(file, strFileName, &pData))
#endif
				return Py_BuildValue(szFormat, pData, file.Size());
		}
	}

	return Py_BuildNone();
}

void initpack()
{
	static PyMethodDef s_methods[] =
	{
		{ "Exist",			packExist,			METH_VARARGS },
		{ "ExistInPack",	packExistInPack,	METH_VARARGS },
		{ "Get",			packGet,			METH_VARARGS },
		{ NULL, NULL },
	};

	Py_InitModule(CPythonDynamicModule::Instance().GetModule(PYTHON_PACK).c_str(), s_methods);
}