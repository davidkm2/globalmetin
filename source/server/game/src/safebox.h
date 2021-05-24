#ifndef __INC_METIN_II_GAME_SAFEBOX_H__
#define __INC_METIN_II_GAME_SAFEBOX_H__

class CHARACTER;
class CItem;

class CSafebox
{
	public:
		CSafebox(LPCHARACTER pkChrOwner, int iSize, DWORD dwGold);
		~CSafebox();

		bool		Add(DWORD dwPos, LPITEM pkItem);
		LPITEM		Get(DWORD dwPos);
		LPITEM		Remove(DWORD dwPos);

		bool		MoveItem(DWORD dwCell, DWORD dwDestCell, DWORD count);
		LPITEM		GetItem(DWORD dwCell);

		void		Save();

		bool		IsEmpty(WORD wCell, BYTE bSize, int iExceptionCell = -1);
		bool		IsValidPosition(DWORD dwPos);

		void		SetWindowMode(BYTE bWindowMode);

		int			GetEmptySafeBox(DWORD dwSize);
	protected:
		void		__Destroy();

		LPCHARACTER	m_pkChrOwner;
		LPITEM		m_pkItems[SAFEBOX_MAX_NUM];
		WORD		m_pkGrid[SAFEBOX_MAX_NUM];
		long		m_lGold;

		BYTE		m_bWindowMode;
};

#endif

