#ifndef __INC_METIN_II_GAME_LOCALIZATION_SYSTEM_H__
#define __INC_METIN_II_GAME_LOCALIZATION_SYSTEM_H__
#include <sstream>
#pragma once

class LocalizationManager : public singleton<LocalizationManager>
{
	public:
		LocalizationManager();
		~LocalizationManager();

	bool	Send_Localization_Open(LPCHARACTER ch, bool refresh = false);
	bool	Send_Localization_Save(LPCHARACTER ch, int tmpIndex);
	bool	Send_Localization_Teleport(LPCHARACTER ch, int tmpIndex);
};
#endif

