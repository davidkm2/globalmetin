#include "StdAfx.h"
#include "PythonStackCheck.h"
#include "CheatQueueManager.h"

#include <XORstr.h>
#include <Python/Python.h>
#include <Python/frameobject.h>

#pragma optimize("", off )
CPythonStackController::CPythonStackController()
{
}

CPythonStackController::~CPythonStackController()
{
}

std::string CPythonStackController::GetReferenceFileName()
{
	PyThreadState * ts = PyThreadState_Get();
	PyFrameObject* frame = ts->frame;

	char const* c_szCallerFileName = PyString_AsString(frame->f_code->co_filename);
	return c_szCallerFileName;
}

std::string CPythonStackController::GetReferenceFunctionName()
{
	PyThreadState * ts = PyThreadState_Get();
	PyFrameObject* frame = ts->frame;

	char const* c_szCallerFunctionName = PyString_AsString(frame->f_code->co_name);
	return c_szCallerFunctionName;
}


void CPythonStackController::SendViolationMessage(int32_t iTypeId, const std::string & szRefFileName, const std::string & szRefFuncName)
{
#ifdef _DEBUG
	TraceError("CPythonStackController::SendViolationMessage; Call is not legal! Type: '%d' File: '%s' Function: '%s'", iTypeId, szRefFileName.c_str(), szRefFuncName.c_str());
#endif

	char szMsg[1024];
	sprintf_s(szMsg, XOR("%d)%s!%s"), iTypeId, szRefFileName.c_str(), szRefFuncName.c_str());

	CCheatDetectQueueMgr::Instance().AppendDetection(CHEAT_PY_API_UNKNOWN_REFERENCE, 0, szMsg);
}

void CPythonStackController::CheckStackReferenceEx(int32_t iTypeId, const std::string & szRefFileName, const std::string & szRefFuncName)
{
	auto strLowerRefFileName = szRefFileName;
	std::transform(strLowerRefFileName.begin(), strLowerRefFileName.end(), strLowerRefFileName.begin(), tolower);
	
	switch (iTypeId)
	{
		case CHEAT_TYPE_pack_Get:
		{
			if (szRefFileName.empty() || 
				(strLowerRefFileName != "system.py" && strLowerRefFileName != "system_python.py"))
			{
				SendViolationMessage(iTypeId, szRefFileName, szRefFuncName);
				return;
			}
		} break;

		default:
			break;
	}
}

void CPythonStackController::CheckStackReference(int32_t iTypeId, const std::string & szRefFileName, const std::string & szRefFuncName)
{
	if (!iTypeId || szRefFileName.empty() || szRefFuncName.empty())
		return;

	switch (iTypeId)
	{
#ifdef _DEBUG
		TraceError("CPythonStackController::CheckStackReference - Type: '%d' File: '%s' Function: '%s'", iTypeId, szRefFileName.c_str(), szRefFuncName.c_str());
#endif

		case CHEAT_TYPE_chat_AppendChat:
		case CHEAT_TYPE_chat_AppendChatWithDelay:
		case CHEAT_TYPE_chat_AppendWhisper:
		case CHEAT_TYPE_chat_GetLinkFromHyperlink:
		case CHEAT_TYPE_nonplayer_GetMonsterName:
		case CHEAT_TYPE_nonplayer_GetLevelByVID:
		case CHEAT_TYPE_background_GlobalPositionToMapInfo:
		case CHEAT_TYPE_background_GetCurrentMapName:
		case CHEAT_TYPE_background_EnableSnow:
		case CHEAT_TYPE_background_SetEnvironmentData:
		case CHEAT_TYPE_background_RegisterEnvironmentData:
		case CHEAT_TYPE_background_LoadMap:
		case CHEAT_TYPE_shop_AddPrivateShopItemStock:
		case CHEAT_TYPE_shop_BuildPrivateShop:
		case CHEAT_TYPE_shop_ClearPrivateShopStock:
		case CHEAT_TYPE_shop_GetItemPrice:
		case CHEAT_TYPE_shop_GetItemMetinSocket:
		case CHEAT_TYPE_shop_GetItemCount:
		case CHEAT_TYPE_shop_GetItemID:
		case CHEAT_TYPE_ime_PasteString:
		case CHEAT_TYPE_event_SelectAnswer:
		case CHEAT_TYPE_event_QuestButtonClick:
		case CHEAT_TYPE_skill_IsToggleSkill:
		case CHEAT_TYPE_skill_GetSkillName:
		case CHEAT_TYPE_skill_CanUseSkill:
		case CHEAT_TYPE_pack_Get:
			return CheckStackReferenceEx(iTypeId, szRefFileName.c_str(), szRefFuncName.c_str());

		default:
			break;
	}

#ifdef _DEBUG
	TraceError("CPythonStackController::CheckStackReference Error - Unknown type: %d", iTypeId);
#endif
}

#pragma optimize("", on )

