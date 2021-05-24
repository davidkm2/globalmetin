#pragma once
#include "../../common/tables.h"

class CMailBoxManager : public singleton<CMailBoxManager>
{
	public:
		CMailBoxManager(void);
		virtual ~CMailBoxManager(void);
		void Initialize(void);

		void Close(LPCHARACTER ch);
		void WriteConfirm(LPCHARACTER ch, const char * szName);
		void Write(LPCHARACTER ch, const char * szName, const char * szTitle, const char * szMessage, BYTE bWindowType, DWORD dwItemPos, DWORD dwMoney, DWORD dwCheque);
		void PostGetItems(LPCHARACTER ch, BYTE bDataIndex);
		void PostAddData(LPCHARACTER ch, BYTE bButtonIndex, BYTE bDataIndex);
		void PostDelete(LPCHARACTER ch, BYTE bDataIndex);
		void PostAllDelete(LPCHARACTER ch);
		void PostAllGetItems(LPCHARACTER ch);
		void CheckMails(LPCHARACTER ch);

	protected:
};

