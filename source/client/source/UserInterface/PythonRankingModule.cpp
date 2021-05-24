#include "StdAfx.h"
#include "PythonRanking.h"
#include "Packet.h"

PyObject * rankingGetHighRankingInfoSolo(PyObject* poSelf, PyObject* poArgs)
{
	int iCategory, iLine;

	if (!PyTuple_GetInteger(poArgs, 0, &iCategory))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &iLine))
		return Py_BuildException();

	const TRankingInfo& Info = CPythonRanking::Instance().GetHighRankingInfo(iCategory, iLine);
	return Py_BuildValue("siiii", Info.szCharName, Info.iRecord0, Info.iRecord1, Info.iTime, Info.bEmpire);
}

PyObject * rankingGetMyRankingInfoSolo(PyObject* poSelf, PyObject* poArgs)
{
	int iCategory;

	if (!PyTuple_GetInteger(poArgs, 0, &iCategory))
		return Py_BuildException();

	const TRankingInfo& Info = CPythonRanking::Instance().GetMyRankingInfo(iCategory);
	return Py_BuildValue("isiiii", Info.iRankingIdx, Info.szCharName, Info.iRecord0, Info.iRecord1, Info.iTime, Info.bEmpire);
}

PyObject * rankingGetHighRankingInfoParty(PyObject* poSelf, PyObject* poArgs)
{
	int iCategory, iLine;

	if (!PyTuple_GetInteger(poArgs, 0, &iCategory))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &iLine))
		return Py_BuildException();

	const TRankingInfo& Info = CPythonRanking::Instance().GetHighRankingInfo(iCategory + 2, iLine);
	return Py_BuildValue("siiii", Info.szCharName, Info.iRecord0, Info.iRecord1, Info.iTime, Info.bEmpire);
}

PyObject * rankingGetMyRankingInfoParty(PyObject* poSelf, PyObject* poArgs)
{
	int iCategory;

	if (!PyTuple_GetInteger(poArgs, 0, &iCategory))
		return Py_BuildException();

	const TRankingInfo& Info = CPythonRanking::Instance().GetMyRankingInfo(iCategory);
	return Py_BuildValue("isiiii", Info.iRankingIdx, Info.szCharName, Info.iRecord0, Info.iRecord1, Info.iTime, Info.bEmpire);
}

PyObject * rankingGetPartyMemberName(PyObject* poSelf, PyObject* poArgs)
{
	int iCategory, iLine;

	if (!PyTuple_GetInteger(poArgs, 0, &iCategory))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &iLine))
		return Py_BuildException();

	return Py_BuildValue("s", CPythonRanking::Instance().GetPartyMemberName(iCategory, iLine));
}

PyObject * rankingGetMyPartyMemberName(PyObject* poSelf, PyObject* poArgs)
{
	int iCategory;

	if (!PyTuple_GetInteger(poArgs, 0, &iCategory))
		return Py_BuildException();

	return Py_BuildValue("s", CPythonRanking::Instance().GetMyPartyMemberName(iCategory));
}

void initRanking()
{
	static PyMethodDef s_methods[] =
	{
		{ "GetHighRankingInfoSolo",				rankingGetHighRankingInfoSolo,			METH_VARARGS },
		{ "GetMyRankingInfoSolo",				rankingGetMyRankingInfoSolo,			METH_VARARGS },
		{ "GetHighRankingInfoParty",			rankingGetHighRankingInfoParty,			METH_VARARGS },
		{ "GetMyRankingInfoParty",				rankingGetMyRankingInfoParty,			METH_VARARGS },
		{ "GetPartyMemberName",					rankingGetPartyMemberName,				METH_VARARGS },
		{ "GetMyPartyMemberName",				rankingGetMyPartyMemberName,			METH_VARARGS },
		{ NULL,									NULL,									NULL },
	};

	PyObject * poModule = Py_InitModule("ranking", s_methods);

	PyModule_AddIntConstant(poModule, "TYPE_RK_SOLO",											TYPE_RK_SOLO);
	PyModule_AddIntConstant(poModule, "TYPE_RK_PARTY",											TYPE_RK_PARTY);
	PyModule_AddIntConstant(poModule, "TYPE_RK_MAX",											TYPE_RK_MAX);

	PyModule_AddIntConstant(poModule, "SOLO_RK_CATEGORY_BF_WEAK",								SOLO_RK_CATEGORY_BF_WEAK);
	PyModule_AddIntConstant(poModule, "SOLO_RK_CATEGORY_BF_TOTAL",								SOLO_RK_CATEGORY_BF_TOTAL);
	PyModule_AddIntConstant(poModule, "SOLO_RK_CATEGORY_MAX",									SOLO_RK_CATEGORY_MAX);

	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_ALL",				PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_ALL);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_NOW_WEEK",		PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_NOW_WEEK);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_PAST_WEEK",		PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_PAST_WEEK);
#ifdef ENABLE_12ZI
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_MOUSE",								PARTY_RK_CATEGORY_CZ_MOUSE);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_COW",								PARTY_RK_CATEGORY_CZ_COW);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_TIGER",								PARTY_RK_CATEGORY_CZ_TIGER);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_RABBIT",							PARTY_RK_CATEGORY_CZ_RABBIT);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_DRAGON",							PARTY_RK_CATEGORY_CZ_DRAGON);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_SNAKE",								PARTY_RK_CATEGORY_CZ_SNAKE);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_HORSE",								PARTY_RK_CATEGORY_CZ_HORSE);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_SHEEP",								PARTY_RK_CATEGORY_CZ_SHEEP);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_MONKEY",							PARTY_RK_CATEGORY_CZ_MONKEY);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_CHICKEN",							PARTY_RK_CATEGORY_CZ_CHICKEN);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_DOG",								PARTY_RK_CATEGORY_CZ_DOG);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_PIG",								PARTY_RK_CATEGORY_CZ_PIG);
#endif
#ifdef ENABLE_DEFENSE_WAVE
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_DEFENSE_WAVE",							PARTY_RK_CATEGORY_DEFENSE_WAVE);
#endif
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_MAX",									PARTY_RK_CATEGORY_MAX);
}

