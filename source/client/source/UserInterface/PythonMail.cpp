#include "StdAfx.h"
#include "PythonMail.h"

CPythonMail::CPythonMail(void)
{
	Clear();
}

CPythonMail::~CPythonMail(void)
{
	Clear();
}

void CPythonMail::Clear(void)
{
	m_MailData.clear();
}

const TItemData * CPythonMail::GetMailItemData(int id) const
{
	if (m_MailData.find(id) != m_MailData.end())
		return &m_MailData.at(id).item;

	return nullptr;
}

DWORD CPythonMail::GetMailItemMetinSocket(int id, int dwMetinSocketIndex)
{
	if (dwMetinSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return 0;

	return GetMailItemData(id)->alSockets[dwMetinSocketIndex];
}

void CPythonMail::GetMailItemAttribute(int id, int dwAttrSlotIndex, BYTE * pbyType, int * psValue)
{
	*pbyType = 0;
	*psValue = 0;

	if (dwAttrSlotIndex >= ITEM_ATTRIBUTE_SLOT_MAX_NUM)
		return;

	*pbyType = GetMailItemData(id)->aAttr[dwAttrSlotIndex].bType;
	*psValue = GetMailItemData(id)->aAttr[dwAttrSlotIndex].sValue;
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
DWORD CPythonMail::GetItemChangeLookVnum(int id)
{
	return GetMailItemData(id)->transmutation;
}
#endif

void CPythonMail::AppendMailData(BYTE bDataIndex, TMailData MData)
{
	m_MailData.insert(make_pair(bDataIndex, MData));
}

const TMailData * CPythonMail::GetMailData(int id) const
{
	if (m_MailData.find(id) != m_MailData.end())
		return &m_MailData.at(id);

	return nullptr;
}

BYTE CPythonMail::GetNotConfirmCount() const
{
	BYTE bCount = 0;
	for (auto it : m_MailData)
		if (!it.second.is_confirm)
			bCount++;

	return bCount;
}

BYTE CPythonMail::GetItemCount() const
{
	BYTE bCount = 0;
	for (auto it : m_MailData)
		if (!it.second.is_confirm && it.second.is_item_exist)
			bCount++;

	return bCount;
}

void CPythonMail::GetItems(BYTE bDataIndex)
{
	m_MailData[bDataIndex].is_item_exist = false;
	m_MailData[bDataIndex].is_confirm = true;
	m_MailData[bDataIndex].yang = 0;
	m_MailData[bDataIndex].cheque = 0;
	memset(&m_MailData[bDataIndex].item, 0, sizeof(m_MailData[bDataIndex].item));
}

void CPythonMail::DeleteMail(BYTE bDataIndex)
{
	m_MailData.erase(bDataIndex);
}

void CPythonMail::AllDeleteMail()
{
	for (BYTE i = 0; i < MAIL_SLOT_MAX_NUM; ++i)
		if (!m_MailData[i].is_item_exist)
			DeleteMail(i);
}

PyObject * mailGetMailItemAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int id, iAttributeSlotIndex, sValue;
	BYTE byType;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &iAttributeSlotIndex))
		return Py_BuildException();

	CPythonMail::Instance().GetMailItemAttribute(id, iAttributeSlotIndex, &byType, &sValue);

	return Py_BuildValue("ii", byType, sValue);
}

PyObject * mailGetMailItemMetinSocket(PyObject* poSelf, PyObject* poArgs)
{
	int id, iMetinSocketIndex;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketIndex))
		return Py_BuildException();

	int nMetinSocketValue = CPythonMail::Instance().GetMailItemMetinSocket(id, iMetinSocketIndex);
	return Py_BuildValue("i", nMetinSocketValue);
}

PyObject * mailGetMailItemData(PyObject* poSelf, PyObject* poArgs)
{
	int id;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	return Py_BuildValue("ii", CPythonMail::Instance().GetMailItemData(id)->vnum, CPythonMail::Instance().GetMailItemData(id)->count);
}

PyObject * mailGetMailAddData(PyObject* poSelf, PyObject* poArgs)
{
	int id;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	const TMailData * Data = CPythonMail::Instance().GetMailData(id);

	return Py_BuildValue("ssii", Data->from_name, Data->message, Data->yang, Data->cheque);
}

PyObject * mailGetMailData(PyObject* poSelf, PyObject* poArgs)
{
	int id;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	const TMailData * Data = CPythonMail::Instance().GetMailData(id);

	return Py_BuildValue("iiisiii", Data->index, Data->send_time, Data->delete_time, Data->title, Data->is_gm_post, Data->is_item_exist, Data->is_confirm);
}

PyObject * mailGetMailDict(PyObject* poSelf, PyObject* poArgs)
{

	std::map<BYTE, TMailData> MailData = CPythonMail::Instance().GetMailData();

	PyObject* dict = PyDict_New();

	for (DWORD i = 0; i < MailData.size(); i++)
		PyDict_SetItem(dict, Py_BuildValue("i", MailData[i].index), Py_BuildValue("iiisiii", MailData[i].index, MailData[i].send_time, MailData[i].delete_time, MailData[i].title, MailData[i].is_gm_post, MailData[i].is_item_exist, MailData[i].is_confirm));

	return dict;
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
PyObject * mailGetItemChangeLookVnum(PyObject* poSelf, PyObject* poArgs)
{
	int id;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	int iChangeLook = CPythonMail::Instance().GetItemChangeLookVnum(id);
	return Py_BuildValue("i", iChangeLook);
}
#endif

void initMail()
{
	static PyMethodDef s_methods[] =
	{
		{ "GetMailItemAttribute",	mailGetMailItemAttribute,	METH_VARARGS },
		{ "GetMailItemMetinSocket",	mailGetMailItemMetinSocket,	METH_VARARGS },
		{ "GetMailItemData",		mailGetMailItemData,		METH_VARARGS },
		{ "GetMailAddData",			mailGetMailAddData,			METH_VARARGS },
		{ "GetMailData",			mailGetMailData,			METH_VARARGS },
		{ "GetMailDict",			mailGetMailDict,			METH_VARARGS },
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		{ "GetItemChangeLookVnum",	mailGetItemChangeLookVnum,	METH_VARARGS },
#endif
		{ NULL,		NULL,		NULL },
	};

	PyObject * poModule = Py_InitModule("mail", s_methods);

	PyModule_AddIntConstant(poModule, "POST_WRITE_FAIL",					POST_WRITE_FAIL);
	PyModule_AddIntConstant(poModule, "POST_WRITE_OK",						POST_WRITE_OK);
	PyModule_AddIntConstant(poModule, "POST_WRITE_INVALID_NAME",			POST_WRITE_INVALID_NAME);
	PyModule_AddIntConstant(poModule, "POST_WRITE_TARGET_BLOCKED",			POST_WRITE_TARGET_BLOCKED);
	PyModule_AddIntConstant(poModule, "POST_WRITE_BLOCKED_ME",				POST_WRITE_BLOCKED_ME);
	PyModule_AddIntConstant(poModule, "POST_WRITE_FULL_MAILBOX",			POST_WRITE_FULL_MAILBOX);
	PyModule_AddIntConstant(poModule, "POST_WRITE_WRONG_TITLE",				POST_WRITE_WRONG_TITLE);
	PyModule_AddIntConstant(poModule, "POST_WRITE_YANG_NOT_ENOUGHT",		POST_WRITE_YANG_NOT_ENOUGHT);
	PyModule_AddIntConstant(poModule, "POST_WRITE_WON_NOT_ENOUGHT",			POST_WRITE_WON_NOT_ENOUGHT);
	PyModule_AddIntConstant(poModule, "POST_WRITE_WRONG_MESSAGE",			POST_WRITE_WRONG_MESSAGE);
	PyModule_AddIntConstant(poModule, "POST_WRITE_WRONG_ITEM",				POST_WRITE_WRONG_ITEM);
	PyModule_AddIntConstant(poModule, "POST_WRITE_LEVEL_NOT_ENOUGHT",		POST_WRITE_LEVEL_NOT_ENOUGHT);

	PyModule_AddIntConstant(poModule, "POST_DELETE_FAIL",					POST_DELETE_FAIL);
	PyModule_AddIntConstant(poModule, "POST_DELETE_OK",						POST_DELETE_OK);
	PyModule_AddIntConstant(poModule, "POST_DELETE_FAIL_EXIST_ITEMS",		POST_DELETE_FAIL_EXIST_ITEMS);

	PyModule_AddIntConstant(poModule, "MAILBOX_GC_OPEN",					MAILBOX_GC_OPEN);
	PyModule_AddIntConstant(poModule, "MAILBOX_GC_POST_WRITE_CONFIRM",		MAILBOX_GC_POST_WRITE_CONFIRM);
	PyModule_AddIntConstant(poModule, "MAILBOX_GC_POST_WRITE",				MAILBOX_GC_POST_WRITE);
	PyModule_AddIntConstant(poModule, "MAILBOX_GC_SET",						MAILBOX_GC_SET);
	PyModule_AddIntConstant(poModule, "MAILBOX_GC_ADD_DATA",				MAILBOX_GC_ADD_DATA);
	PyModule_AddIntConstant(poModule, "MAILBOX_GC_POST_GET_ITEMS",			MAILBOX_GC_POST_GET_ITEMS);
	PyModule_AddIntConstant(poModule, "MAILBOX_GC_POST_DELETE",				MAILBOX_GC_POST_DELETE);
	PyModule_AddIntConstant(poModule, "MAILBOX_GC_POST_ALL_DELETE",			MAILBOX_GC_POST_ALL_DELETE);
	PyModule_AddIntConstant(poModule, "MAILBOX_GC_POST_ALL_GET_ITEMS",		MAILBOX_GC_POST_ALL_GET_ITEMS);
	PyModule_AddIntConstant(poModule, "MAILBOX_GC_UNREAD_DATA",				MAILBOX_GC_UNREAD_DATA);
	PyModule_AddIntConstant(poModule, "MAILBOX_GC_ITEM_EXPIRE",				MAILBOX_GC_ITEM_EXPIRE);
	PyModule_AddIntConstant(poModule, "MAILBOX_GC_CLOSE",					MAILBOX_GC_CLOSE);
	PyModule_AddIntConstant(poModule, "MAILBOX_GC_SYSTEM_CLOSE",			MAILBOX_GC_SYSTEM_CLOSE);

	PyModule_AddIntConstant(poModule, "POST_GET_ITEMS_FAIL",					POST_GET_ITEMS_FAIL);
	PyModule_AddIntConstant(poModule, "POST_GET_ITEMS_OK",						POST_ALL_GET_ITEMS_OK);
	PyModule_AddIntConstant(poModule, "POST_GET_ITEMS_NONE",					POST_ALL_GET_ITEMS_EMPTY);
	PyModule_AddIntConstant(poModule, "POST_GET_ITEMS_NOT_ENOUGHT_INVENTORY",	POST_ALL_GET_ITEMS_FAIL_DONT_EXIST);
	PyModule_AddIntConstant(poModule, "POST_GET_ITEMS_YANG_OVERFLOW",			POST_ALL_GET_ITEMS_FAIL_CANT_GET);
	PyModule_AddIntConstant(poModule, "POST_GET_ITEMS_WON_OVERFLOW",			POST_ALL_GET_ITEMS_FAIL_CANT_GET);
	PyModule_AddIntConstant(poModule, "POST_GET_ITEMS_FAIL_BLOCK_CHAR",			POST_ALL_GET_ITEMS_FAIL_CANT_GET);

	PyModule_AddIntConstant(poModule, "POST_ALL_DELETE_FAIL",				POST_ALL_DELETE_FAIL);
	PyModule_AddIntConstant(poModule, "POST_ALL_DELETE_OK",					POST_ALL_DELETE_OK);
	PyModule_AddIntConstant(poModule, "POST_ALL_DELETE_FAIL_EMPTY",			POST_ALL_DELETE_FAIL_EMPTY);
	PyModule_AddIntConstant(poModule, "POST_ALL_DELETE_FAIL_DONT_EXIST",	POST_ALL_DELETE_FAIL_DONT_EXIST);

	PyModule_AddIntConstant(poModule, "POST_ALL_GET_ITEMS_FAIL",			POST_ALL_GET_ITEMS_FAIL);
	PyModule_AddIntConstant(poModule, "POST_ALL_GET_ITEMS_OK",				POST_ALL_GET_ITEMS_OK);
	PyModule_AddIntConstant(poModule, "POST_ALL_GET_ITEMS_EMPTY",			POST_ALL_GET_ITEMS_EMPTY);
	PyModule_AddIntConstant(poModule, "POST_ALL_GET_ITEMS_FAIL_DONT_EXIST",	POST_ALL_GET_ITEMS_FAIL_DONT_EXIST);
	PyModule_AddIntConstant(poModule, "POST_ALL_GET_ITEMS_FAIL_CANT_GET",	POST_ALL_GET_ITEMS_FAIL_CANT_GET);

	PyModule_AddIntConstant(poModule, "POST_TIME_OUT_DELETE",				POST_TIME_OUT_DELETE);
}

