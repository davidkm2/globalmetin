#include "stdafx.h"
#include "PythonMessenger.h"

void CPythonMessenger::RemoveFriend(const char * c_szKey)
{
	m_FriendNameMap.erase(c_szKey);
}

void CPythonMessenger::OnFriendLogin(const char * c_szKey)
{
	m_FriendNameMap.insert(c_szKey);

	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnLogin", Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_FRIEND, c_szKey));
}

void CPythonMessenger::OnFriendLogout(const char * c_szKey)
{
	m_FriendNameMap.insert(c_szKey);

	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout", Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_FRIEND, c_szKey));
}

#ifdef ENABLE_MESSENGER_BLOCK
void CPythonMessenger::OnBlockLogin(const char * c_szKey)
{
	m_BlockNameMap.insert(c_szKey);
		PyCallClassMemberFunc(m_poMessengerHandler, "OnLogin", Py_BuildValue("(is)", MESSENGER_GROUP_INDEX_BLOCK, c_szKey));
}

void CPythonMessenger::OnBlockLogout(const char * c_szKey)
{
	m_BlockNameMap.insert(c_szKey);

	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout", Py_BuildValue("(is)", MESSENGER_GROUP_INDEX_BLOCK, c_szKey));
}
BOOL CPythonMessenger::IsBlockByKey(const char * c_szKey)
{
	return (m_BlockNameMap.end() != m_BlockNameMap.find(c_szKey));
}

BOOL CPythonMessenger::IsBlockFriendByName(const char * c_szName)
{
	return IsBlockByKey(c_szName);
}
void CPythonMessenger::RemoveBlock(const char * c_szKey)
{
	m_BlockNameMap.erase(c_szKey);
	PyCallClassMemberFunc(m_poMessengerHandler, "OnRemoveList", Py_BuildValue("(is)", MESSENGER_GROUP_INDEX_BLOCK, c_szKey));
}
#endif

BOOL CPythonMessenger::IsFriendByKey(const char * c_szKey)
{
	return m_FriendNameMap.end() != m_FriendNameMap.find(c_szKey);
}

BOOL CPythonMessenger::IsFriendByName(const char * c_szName)
{
	return IsFriendByKey(c_szName);
}

void CPythonMessenger::AppendGuildMember(const char * c_szName)
{
	if (m_GuildMemberStateMap.end() != m_GuildMemberStateMap.find(c_szName))
		return;

	LogoutGuildMember(c_szName);
}

void CPythonMessenger::RemoveGuildMember(const char * c_szName)
{
	m_GuildMemberStateMap.erase(c_szName);

	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnRemoveList", Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, c_szName));
}

void CPythonMessenger::RemoveAllGuildMember()
{
	m_GuildMemberStateMap.clear();

	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnRemoveAllList", Py_BuildValue("(i)", MESSENGER_GRUOP_INDEX_GUILD));
}

void CPythonMessenger::LoginGuildMember(const char * c_szName)
{
	m_GuildMemberStateMap[c_szName] = 1;
	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnLogin", Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, c_szName));
}

void CPythonMessenger::LogoutGuildMember(const char * c_szName)
{
	m_GuildMemberStateMap[c_szName] = 0;
	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout", Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, c_szName));
}

void CPythonMessenger::RefreshGuildMember()
{
	for (TGuildMemberStateMap::iterator itor = m_GuildMemberStateMap.begin(); itor != m_GuildMemberStateMap.end(); ++itor)
	{
		if (itor->second)
			PyCallClassMemberFunc(m_poMessengerHandler, "OnLogin", Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, (itor->first).c_str()));
		else
			PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout", Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, (itor->first).c_str()));
	}
}

void CPythonMessenger::Destroy()
{
	m_FriendNameMap.clear();
	m_GuildMemberStateMap.clear();
}

void CPythonMessenger::SetMessengerHandler(PyObject* poHandler)
{
	m_poMessengerHandler = poHandler;
}

CPythonMessenger::CPythonMessenger()
	: m_poMessengerHandler(NULL)
{
}

CPythonMessenger::~CPythonMessenger()
{
}
PyObject * messengerRemoveFriend(PyObject* poSelf, PyObject* poArgs)
{
	char * szKey;
	if (!PyTuple_GetString(poArgs, 0, &szKey))
		return Py_BuildException();

	CPythonMessenger::Instance().RemoveFriend(szKey);
	return Py_BuildNone();
}

PyObject * messengerIsFriendByName(PyObject* poSelf, PyObject* poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonMessenger::Instance().IsFriendByName(szName));
}

#ifdef ENABLE_MESSENGER_BLOCK
PyObject * messengerIsBlockFriendByName(PyObject* poSelf, PyObject* poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();
	
	return Py_BuildValue("i", CPythonMessenger::Instance().IsBlockFriendByName(szName));
}
PyObject * messengerRemoveBlock(PyObject* poSelf, PyObject* poArgs)
{
	char * szKey;
	if (!PyTuple_GetString(poArgs, 0, &szKey))
		return Py_BuildException();

	CPythonMessenger::Instance().RemoveBlock(szKey);
	return Py_BuildNone();
}
#endif

PyObject * messengerDestroy(PyObject* poSelf, PyObject* poArgs)
{
	CPythonMessenger::Instance().Destroy();
	return Py_BuildNone();
}

PyObject * messengerRefreshGuildMember(PyObject* poSelf, PyObject* poArgs)
{
	CPythonMessenger::Instance().RefreshGuildMember();
	return Py_BuildNone();
}

PyObject * messengerSetMessengerHandler(PyObject* poSelf, PyObject* poArgs)
{
	PyObject * poEventHandler;
	if (!PyTuple_GetObject(poArgs, 0, &poEventHandler))
		return Py_BuildException();

	CPythonMessenger::Instance().SetMessengerHandler(poEventHandler);
	return Py_BuildNone();
}

#ifdef ENABLE_MAILBOX
PyObject * messengerGetFriendNames(PyObject* poSelf, PyObject* poArgs)
{
	CPythonMessenger::TFriendNameMap FriendNameMap = CPythonMessenger::Instance().GetFriendMap();

	PyObject* Names = PyTuple_New(FriendNameMap.size());

	int i = 0;
	for (CPythonMessenger::TFriendNameMap::iterator it = FriendNameMap.begin(); FriendNameMap.end() != it; ++it)
		PyTuple_SetItem(Names, i++, PyString_FromString((*it).c_str()));

	return Names;
}
#endif

void initMessenger()
{
	static PyMethodDef s_methods[] =
	{
		{ "RemoveFriend",				messengerRemoveFriend,				METH_VARARGS },
		{ "IsFriendByName",				messengerIsFriendByName,			METH_VARARGS } ,
		{ "Destroy",					messengerDestroy,					METH_VARARGS },
		{ "RefreshGuildMember",			messengerRefreshGuildMember,		METH_VARARGS },
		{ "SetMessengerHandler",		messengerSetMessengerHandler,		METH_VARARGS },
#ifdef ENABLE_MESSENGER_BLOCK
		{ "IsBlockFriendByName",		messengerIsBlockFriendByName,		METH_VARARGS },
		{ "RemoveBlock",				messengerRemoveBlock,				METH_VARARGS },
#endif
#ifdef ENABLE_MAILBOX
		{ "GetFriendNames",				messengerGetFriendNames,			METH_VARARGS },
#endif
		{ NULL,							NULL,								NULL         },
	};

	Py_InitModule("messenger", s_methods);
}
