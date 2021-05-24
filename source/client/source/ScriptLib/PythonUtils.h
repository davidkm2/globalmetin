#pragma once

#define SET_EXCEPTION(x) PyErr_SetString(PyExc_RuntimeError, #x)

bool PyTuple_GetString(PyObject* poArgs, int32_t pos, char** ret);
bool PyTuple_GetInteger(PyObject* poArgs, int32_t pos, uint8_t* ret);
bool PyTuple_GetInteger(PyObject* poArgs, int32_t pos, int32_t* ret);
bool PyTuple_GetInteger(PyObject* poArgs, int32_t pos, uint16_t* ret);
bool PyTuple_GetByte(PyObject* poArgs, int32_t pos, uint8_t* ret);
bool PyTuple_GetUnsignedInteger(PyObject* poArgs, int32_t pos, uint32_t* ret);
bool PyTuple_GetLong(PyObject* poArgs, int32_t pos, int32_t* ret);
bool PyTuple_GetLongLong(PyObject* poArgs, int pos, long long* ret);
bool PyTuple_GetUnsignedLong(PyObject* poArgs, int32_t pos, uint32_t* ret);
bool PyTuple_GetUnsignedLongLong(PyObject* poArgs, int32_t pos, uint64_t* ret);
bool PyTuple_GetFloat(PyObject* poArgs, int32_t pos, float* ret);
bool PyTuple_GetDouble(PyObject* poArgs, int32_t pos, double* ret);
bool PyTuple_GetObject(PyObject* poArgs, int32_t pos, PyObject** ret);
bool PyTuple_GetBoolean(PyObject* poArgs, int32_t pos, bool* ret);

bool PyCallClassMemberFunc(PyObject* poClass, const char* c_szFunc, PyObject* poArgs);
bool PyCallClassMemberFunc(PyObject* poClass, const char* c_szFunc, PyObject* poArgs, bool* pisRet);
bool PyCallClassMemberFunc(PyObject* poClass, const char* c_szFunc, PyObject* poArgs, int32_t * plRetValue);

bool PyCallClassMemberFunc_ByPyString(PyObject* poClass, PyObject* poFuncName, PyObject* poArgs);
bool PyCallClassMemberFunc(PyObject* poClass, PyObject* poFunc, PyObject* poArgs);

#define Py_BuildException(...) __Py_BuildException(__FUNCTION__, __LINE__, __VA_ARGS__)
PyObject * __Py_BuildException(const char* c_pszFunc, int32_t iLine, const char * c_pszErr = nullptr, ...);
PyObject * Py_BadArgument();
PyObject * Py_BuildNone();
PyObject * Py_BuildEmptyTuple();
