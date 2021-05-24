#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "NTDefinations.h"
#include "../eterPack/EterPackManager.h"

#include <XORstr.h>
#include <Python/Python.h>

#if 0
// TODO: hook
PyAPI_FUNC(PyObject *) Py_InitModule4(const char *name, PyMethodDef *methods, const char *doc, PyObject *self, int apiver);
PyAPI_FUNC(PyObject *) PyString_InternFromString(const char *);
PyAPI_FUNC(struct _mod *) PyParser_ASTFromString(const char *, const char *, int, PyCompilerFlags *flags,  PyArena *);
PyAPI_FUNC(struct _mod *) PyParser_ASTFromFile(FILE *, const char *, int, char *, char *, PyCompilerFlags *, int *,  PyArena *);
#endif

PyObject* GetModuleByName(const std::string& stModuleName)
{
	PyObject *modules = PyImport_GetModuleDict();

    Py_ssize_t pos = 0;
	PyObject *mod_name = nullptr, *mod = nullptr;
    while (PyDict_Next(modules, &pos, &mod_name, &mod))
	{
		auto stCurrModuleName = std::string(PyString_AsString(mod_name));
		if (stModuleName == stCurrModuleName)
			break;
	}
	return mod;
}

auto PyTupleToStdVector(PyObject* pyTupleObj)
{
	std::vector <std::string> vBuffer;

	if (PyTuple_Check(pyTupleObj))
	{
		for (Py_ssize_t i = 0; i < PyTuple_Size(pyTupleObj); ++i)
		{
			PyObject *value = PyTuple_GetItem(pyTupleObj, i);
			vBuffer.push_back(PyString_AsString(value));
		}
	}
	return vBuffer;
}

static const std::vector <std::string> gsc_vPythonApiModules = // TODO: Create a global vector and store Py_InitModule returned objects instead storing names
{
	"grpImage",
	"grp",
	"grpText",
	"grpThing",
	"wndMgr",
	"dbg",
	"acce",
	"app",
	"background",
	"changelook",
	"chrmgr",
	"chr",
	"chat",
	"impMgr",
	"effect",
	"event",
	"exchange",
	"fly",
	"eventMgr",
	"guild",
	"ime",
	"item",
	"mail",
	"messenger",
	"miniMap",
	"net",
	"nonplayer",
	"ops",
	"pack",
	"player",
	"profiler",
	"quest",
	"ranking",
	"safebox",
	"shop",
	"skill",
	"petskill",
	"snd",
	"systemSetting",
	"textTail",
	"ServerStateChecker"
};

std::vector <std::string> gs_vBuiltinModules;

bool InitializeBuiltinModuleList() // TODO: init
{
	auto pySysModule = GetModuleByName("sys");
	if (!pySysModule)
		return false;

	if (!PyObject_HasAttrString(pySysModule, "builtin_module_names"))
		return false;

	auto pBuiltinModules = PyObject_GetAttrString(pySysModule, "builtin_module_names");
	if (!pBuiltinModules || pBuiltinModules == Py_None)
		return false;

	gs_vBuiltinModules = PyTupleToStdVector(pBuiltinModules);
	return true;
}

void CheckPythonModules() // TODO: check just one time before than initilization of Py_InitModule hook, add detection log funcs
{
	PyObject *modules = PyImport_GetModuleDict();

    Py_ssize_t pos = 0;
	PyObject *mod_name, *mod;
    while (PyDict_Next(modules, &pos, &mod_name, &mod))
	{
		auto stModuleName = std::string(PyString_AsString(mod_name));
//		TraceError("%s", stModuleName.c_str());

		if (PyObject_HasAttrString(mod, "__file__")) // file
		{
			auto pyModuleFileName  = PyObject_GetAttrString(mod, "__file__");
			if (!pyModuleFileName || pyModuleFileName == Py_None)
			{
				TraceError("unkown file-py: %s", stModuleName.c_str());
				continue;
			}

			auto stCurrModuleFile = std::string(PyString_AsString(pyModuleFileName));
#ifdef ENABLE_FOX_FS
			if (CFileSystem::Instance().isExistInPack(stCurrModuleFile.c_str(), __FUNCTION__) == false)
#else
			if (CEterPackManager::Instance().isExistInPack(stCurrModuleFile.c_str()) == false)
#endif
			{
				TraceError("suspected file: %s", stCurrModuleFile.c_str());				
			}
		}
		else // builtin
		{
			if (std::find(gs_vBuiltinModules.begin(), gs_vBuiltinModules.end(), stModuleName) == gs_vBuiltinModules.end() && // have not exist in sys module's builtin module container
				std::find(gsc_vPythonApiModules.begin(), gsc_vPythonApiModules.end(), stModuleName) == gsc_vPythonApiModules.end()) // does not created by client's python api
			{
				TraceError("suspected builtin: %s", stModuleName.c_str());
			}
		}
	}
}