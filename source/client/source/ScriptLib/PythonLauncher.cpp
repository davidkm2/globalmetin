#include "StdAfx.h"
#include <Python/frameobject.h>
#include "../eterPack/EterPackManager.h"

#include "PythonLauncher.h"

CPythonLauncher::CPythonLauncher() :
	m_poModule(nullptr), m_poDic(nullptr)
{
	Py_NoSiteFlag++;
	Py_OptimizeFlag++;
}

CPythonLauncher::~CPythonLauncher()
{
	Clear();
}

void CPythonLauncher::Clear()
{
	Py_Finalize();
}

bool CPythonLauncher::Create(const char* c_szProgramName)
{
	Py_SetProgramName(const_cast<char*> (c_szProgramName));
	Py_Initialize();

	m_poModule = PyImport_AddModule("__main__");
	if (!m_poModule)
		return false;

	m_poDic = PyModule_GetDict(m_poModule);

	PyObject* builtins = PyImport_ImportModule("__builtin__");
	PyModule_AddIntConstant(builtins, "TRUE", 1);
	PyModule_AddIntConstant(builtins, "FALSE", 0);
	PyDict_SetItemString(m_poDic, "__builtins__", builtins);
	Py_DECREF(builtins);

	return true;
}

bool CPythonLauncher::RunFile(const std::string& filename, const std::string& modName) const
{
	LPCVOID pData;
	CMappedFile file;
#ifdef ENABLE_FOX_FS
	if (!CFileSystem::Instance().Get(file, filename.c_str(), &pData, __FUNCTION__, true))
#else
	if (!CEterPackManager::Instance().Get(file, filename.c_str(), &pData))
#endif
	{
		TraceError("Get %s failed", filename.c_str());
		return false;
	}

	std::string data(static_cast<const char*>(file.Get()), file.Size());
	if (data.empty())
	{
		TraceError("Data assign for %s failed", filename.c_str());
		return false;
	}

	auto code = Py_CompileString(data.c_str(), filename.c_str(), Py_file_input);
	if (!code)
	{
		TraceError("Compiling %s failed", filename.c_str());
		PyErr_Print();
		return false;
	}

	auto mod = PyImport_ExecCodeModule(const_cast<char*>(modName.c_str()), code);
	if (!mod)
	{
		TraceError("Module creation for %s failed", filename.c_str());
		PyErr_Print();
		return false;
	}

	return true;
}
