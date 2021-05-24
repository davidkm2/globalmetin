#include "StdAfx.h"
#include "PythonUtils.h"

#define PyLong_AsLong PyLong_AsLongLong
#define PyLong_AsUnsignedLong PyLong_AsUnsignedLongLong

bool __PyCallClassMemberFunc_ByCString(PyObject * poClass, const char * c_szFunc, PyObject * poArgs, PyObject ** ppoRet);
bool __PyCallClassMemberFunc_ByPyString(PyObject * poClass, PyObject * poFuncName, PyObject * poArgs, PyObject ** ppoRet);
bool __PyCallClassMemberFunc(PyObject * poClass, PyObject * poFunc, PyObject * poArgs, PyObject ** ppoRet);

PyObject * Py_BadArgument()
{
	PyErr_BadArgument();
	return nullptr;
}

PyObject* __Py_BuildException(const char* c_pszFunc, int32_t iLine, const char* c_pszErr, ...)
{
	if (!c_pszErr)
	{
		TraceError("%s [%d]: Unknown PythonException", c_pszFunc, iLine);
		PyErr_Clear();
	}
	else
	{
		char szErrBuf[512+1];
		va_list args;
		va_start(args, c_pszErr);
		vsnprintf(szErrBuf, sizeof(szErrBuf), c_pszErr, args);
		va_end(args);

		TraceError("%s [%d]: PythonException: %s", c_pszFunc, iLine, szErrBuf);
		PyErr_SetString(PyExc_RuntimeError, szErrBuf);
	}

	return Py_BuildNone();

}

PyObject * Py_BuildNone()
{
	Py_INCREF(Py_None);
	return Py_None;
}

void Py_ReleaseNone()
{
	Py_DECREF(Py_None);
}

bool PyTuple_GetObject(PyObject* poArgs, int32_t pos, PyObject** ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject * poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = poItem;
	return true;
}

bool PyTuple_GetLong(PyObject* poArgs, int32_t pos, int32_t* ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject* poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = PyLong_AsLong(poItem);
	return true;
}

bool PyTuple_GetDouble(PyObject* poArgs, int32_t pos, double* ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject* poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = PyFloat_AsDouble(poItem);
	return true;
}

bool PyTuple_GetFloat(PyObject* poArgs, int32_t pos, float* ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject * poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = static_cast<float>(PyFloat_AsDouble(poItem));
	return true;
}

bool PyTuple_GetByte(PyObject* poArgs, int32_t pos, uint8_t* ret)
{
	int32_t val;
	bool result = PyTuple_GetInteger(poArgs,pos,&val);
	*ret = static_cast<uint8_t>(val);
	return result;
}

bool PyTuple_GetInteger(PyObject* poArgs, int32_t pos, uint8_t* ret)
{
	int32_t val;
	bool result = PyTuple_GetInteger(poArgs,pos,&val);
	*ret = static_cast<uint8_t>(val);
	return result;
}

bool PyTuple_GetInteger(PyObject* poArgs, int32_t pos, uint16_t* ret)
{
	int32_t val;
	bool result = PyTuple_GetInteger(poArgs,pos,&val);
	*ret = static_cast<uint16_t>(val);
	return result;
}

bool PyTuple_GetInteger(PyObject* poArgs, int32_t pos, int32_t* ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject* poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = PyLong_AsLong(poItem);
	return true;
}

bool PyTuple_GetUnsignedLong(PyObject* poArgs, int32_t pos, uint32_t* ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject * poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = PyLong_AsUnsignedLong(poItem);
	return true;
}

bool PyTuple_GetLongLong(PyObject* poArgs, int pos, long long* ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject* poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = PyLong_AsLongLong(poItem);
	return true;
}

bool PyTuple_GetUnsignedLongLong(PyObject* poArgs, int32_t pos, uint64_t* ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject * poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = PyLong_AsUnsignedLongLong(poItem);
	return true;
}

bool PyTuple_GetUnsignedInteger(PyObject* poArgs, int32_t pos, uint32_t* ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject* poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = PyLong_AsUnsignedLong(poItem);
	return true;
}

bool PyTuple_GetString(PyObject* poArgs, int32_t pos, char** ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject* poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	if (!PyString_Check(poItem))
		return false;

	*ret = PyString_AsString(poItem);
	return true;
}

bool PyTuple_GetBoolean(PyObject* poArgs, int32_t pos, bool* ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject* poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = PyLong_AsLong(poItem) ? true : false;
	return true;
}

bool PyCallClassMemberFunc(PyObject* poClass, PyObject* poFunc, PyObject* poArgs)
{
	PyObject* poRet;

	if (!poClass)
	{
		Py_XDECREF(poArgs);
		return false;
	}

	if (!__PyCallClassMemberFunc(poClass, poFunc, poArgs, &poRet))
		return false;

	Py_DECREF(poRet);
	return true;
}

bool PyCallClassMemberFunc(PyObject* poClass, const char* c_szFunc, PyObject* poArgs)
{
	PyObject* poRet;

	if (!poClass)
	{
		Py_XDECREF(poArgs);
		return false;
	}

	if (!__PyCallClassMemberFunc_ByCString(poClass, c_szFunc, poArgs, &poRet))
		return false;

	Py_DECREF(poRet);
	return true;
}

bool PyCallClassMemberFunc_ByPyString(PyObject* poClass, PyObject* poFuncName, PyObject* poArgs)
{
	PyObject* poRet;

	if (!poClass)
	{
		Py_XDECREF(poArgs);
		return false;
	}

	if (!__PyCallClassMemberFunc_ByPyString(poClass, poFuncName, poArgs, &poRet))
		return false;

	Py_DECREF(poRet);
	return true;
}

bool PyCallClassMemberFunc(PyObject* poClass, const char* c_szFunc, PyObject* poArgs, bool* pisRet)
{
	PyObject* poRet;

	if (!__PyCallClassMemberFunc_ByCString(poClass, c_szFunc, poArgs, &poRet))
		return false;

	if (PyNumber_Check(poRet))
		*pisRet = (PyLong_AsLong(poRet) != 0);
	else
		*pisRet = true;

	Py_DECREF(poRet);
	return true;
}

bool PyCallClassMemberFunc(PyObject* poClass, const char* c_szFunc, PyObject* poArgs, int32_t * plRetValue)
{
	PyObject* poRet;

	if (!__PyCallClassMemberFunc_ByCString(poClass, c_szFunc, poArgs, &poRet))
		return false;

	if (PyNumber_Check(poRet))
	{
		*plRetValue = PyLong_AsLong(poRet);
		Py_DECREF(poRet);
		return true;
	}

	Py_DECREF(poRet);
	return false;
}

bool __PyCallClassMemberFunc_ByCString(PyObject* poClass, const char* c_szFunc, PyObject* poArgs, PyObject** ppoRet)
{
	if (!poClass)
	{
		Py_XDECREF(poArgs);
		return false;
	}

	PyObject * poFunc = PyObject_GetAttrString(poClass, c_szFunc); // New Reference

	if (!poFunc)
	{
		PyErr_Clear();
		Py_XDECREF(poArgs);
		return false;
	}

	if (!PyCallable_Check(poFunc))
	{
		Py_DECREF(poFunc);
		Py_XDECREF(poArgs);
		return false;
	}

	PyObject * poRet = PyObject_CallObject(poFunc, poArgs);

	if (!poRet)
	{
		PyErr_Print();

		Py_DECREF(poFunc);
		Py_XDECREF(poArgs);
		return false;
	}

	*ppoRet = poRet;

	Py_DECREF(poFunc);
	Py_XDECREF(poArgs);
	return true;
}

bool __PyCallClassMemberFunc_ByPyString(PyObject* poClass, PyObject* poFuncName, PyObject* poArgs, PyObject** ppoRet)
{
	if (!poClass)
	{
		Py_XDECREF(poArgs);
		return false;
	}

	PyObject * poFunc = PyObject_GetAttr(poClass, poFuncName);

	if (!poFunc)
	{
		PyErr_Clear();
		Py_XDECREF(poArgs);
		return false;
	}

	if (!PyCallable_Check(poFunc))
	{
		Py_DECREF(poFunc);
		Py_XDECREF(poArgs);
		return false;
	}

	PyObject * poRet = PyObject_CallObject(poFunc, poArgs);

	if (!poRet)
	{
		PyErr_Print();

		Py_DECREF(poFunc);
		Py_XDECREF(poArgs);
		return false;
	}

	*ppoRet = poRet;

	Py_DECREF(poFunc);
	Py_XDECREF(poArgs);
	return true;
}

bool __PyCallClassMemberFunc(PyObject* poClass, PyObject * poFunc, PyObject* poArgs, PyObject** ppoRet)
{
	if (!poClass)
	{
		Py_XDECREF(poArgs);
		return false;
	}

	if (!poFunc)
	{
		PyErr_Clear();
		Py_XDECREF(poArgs);
		return false;
	}

	if (!PyCallable_Check(poFunc))
	{
		Py_DECREF(poFunc);
		Py_XDECREF(poArgs);
		return false;
	}

	PyObject * poRet = PyObject_CallObject(poFunc, poArgs);

	if (!poRet)
	{
		PyErr_Print();
		Py_DECREF(poFunc);
		Py_XDECREF(poArgs);
		return false;
	}

	*ppoRet = poRet;

	Py_DECREF(poFunc);
	Py_XDECREF(poArgs);
	return true;
}
