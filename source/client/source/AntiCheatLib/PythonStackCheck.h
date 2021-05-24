#pragma once
#include "../EterBase/Singleton.h"

enum EPythonCheatTypes
{
	CHEAT_TYPE_NONE,
	CHEAT_TYPE_EMPTY1,
	CHEAT_TYPE_EMPTY2,

	/* 'chat' module functions. */
	CHEAT_TYPE_chat_AppendChat,
	CHEAT_TYPE_chat_AppendChatWithDelay,
	CHEAT_TYPE_chat_AppendWhisper,
	CHEAT_TYPE_chat_GetLinkFromHyperlink,

	/* 'nonplayer' module functions. */
	CHEAT_TYPE_nonplayer_GetMonsterName,
	CHEAT_TYPE_nonplayer_GetLevelByVID,

	/* 'background' module functions. */
	CHEAT_TYPE_background_GlobalPositionToMapInfo,
	CHEAT_TYPE_background_GetCurrentMapName,
	CHEAT_TYPE_background_EnableSnow,
	CHEAT_TYPE_background_SetEnvironmentData,
	CHEAT_TYPE_background_RegisterEnvironmentData,
	CHEAT_TYPE_background_LoadMap,

	/* 'shop' module functions. */
	CHEAT_TYPE_shop_AddPrivateShopItemStock,
	CHEAT_TYPE_shop_BuildPrivateShop,
	CHEAT_TYPE_shop_ClearPrivateShopStock,
	CHEAT_TYPE_shop_GetItemPrice,
	CHEAT_TYPE_shop_GetItemMetinSocket,
	CHEAT_TYPE_shop_GetItemCount,
	CHEAT_TYPE_shop_GetItemID,

	/* 'ime' module functions. */
	CHEAT_TYPE_ime_PasteString,

	/* 'event' module functions. */
	CHEAT_TYPE_event_SelectAnswer,
	CHEAT_TYPE_event_QuestButtonClick,

	/* 'skill' module functions. */
	CHEAT_TYPE_skill_IsToggleSkill,
	CHEAT_TYPE_skill_GetSkillName,
	CHEAT_TYPE_skill_CanUseSkill,

	/* 'pack' module functions. */
	CHEAT_TYPE_pack_Get,

	CHEAT_TYPE_MAX,
};

class CPythonStackController : public CSingleton <CPythonStackController>
{
public:
	CPythonStackController();
	virtual ~CPythonStackController();

	void CheckStackReference(int32_t iTypeId, const std::string & szRefFileName, const std::string & szRefFuncName);

	std::string GetReferenceFileName();
	std::string GetReferenceFunctionName();

protected:
	void CheckStackReferenceEx(int32_t iTypeId, const std::string & szRefFileName, const std::string & szRefFuncName);
	void SendViolationMessage(int32_t iTypeId, const std::string & szRefFileName, const std::string & szRefFuncName);
};

#define PY_REF_FILE CPythonStackController::Instance().GetReferenceFileName()
#define PY_REF_FUNC CPythonStackController::Instance().GetReferenceFunctionName()

