#pragma once
#include "Packet.h"

class CPythonMail : public CSingleton<CPythonMail>
{
	public:
		CPythonMail(void);
		virtual ~CPythonMail(void);
		void Clear(void);
		typedef std::map<BYTE, TMailData> TMail;

		const TItemData * GetMailItemData(int id) const;
		std::map<BYTE, TMailData>& GetMailData() { return m_MailData; }
		DWORD GetMailItemMetinSocket(int id, int dwMetinSocketIndex);
		void GetMailItemAttribute(int id, int dwAttrSlotIndex, BYTE * pbyType, int * psValue);
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		DWORD GetItemChangeLookVnum(int id);
#endif

		void AppendMailData(BYTE bDataIndex, TMailData MData);
		const TMailData * GetMailData(int id) const;

		BYTE GetNotConfirmCount() const;
		BYTE GetItemCount() const;
		void GetItems(BYTE bDataIndex);
		void DeleteMail(BYTE bDataIndex);
		void AllDeleteMail();

	protected:
		TMail m_MailData;
};