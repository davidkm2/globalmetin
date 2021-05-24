#include "stdafx.h"
#include "PythonExchange.h"

void CPythonExchange::SetSelfName(const char *name)
{
	strncpy(m_self.name, name, CHARACTER_NAME_MAX_LEN);
}

void CPythonExchange::SetTargetName(const char *name)
{
	strncpy(m_victim.name, name, CHARACTER_NAME_MAX_LEN);
}

char * CPythonExchange::GetNameFromSelf()
{
	return m_self.name;
}

char * CPythonExchange::GetNameFromTarget()
{
	return m_victim.name;
}

#ifdef ENABLE_LEVEL_IN_TRADE
void CPythonExchange::SetSelfLevel(DWORD level)
{
	m_self.level = level;
}

void CPythonExchange::SetTargetLevel(DWORD level)
{
	m_victim.level = level;
}

DWORD CPythonExchange::GetLevelFromSelf()
{
	return m_self.level;
}

DWORD CPythonExchange::GetLevelFromTarget()
{
	return m_victim.level;
}
#endif

void CPythonExchange::SetElkToTarget(DWORD	elk)
{
	m_victim.elk = elk;
}

void CPythonExchange::SetElkToSelf(DWORD elk)
{
	m_self.elk = elk;
}

DWORD CPythonExchange::GetElkFromTarget()
{
	return m_victim.elk;
}

DWORD CPythonExchange::GetElkFromSelf()
{
	return m_self.elk;
}

#ifdef ENABLE_CHEQUE_SYSTEM
void CPythonExchange::SetChequeToTarget(short cheque)
{
	m_victim.cheque = cheque;
}

void CPythonExchange::SetChequeToSelf(short cheque)
{
	m_self.cheque = cheque;
}

short CPythonExchange::GetChequeFromTarget()
{
	return m_victim.cheque;
}

short CPythonExchange::GetChequeFromSelf()
{
	return m_self.cheque;
}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
void CPythonExchange::SetItemTransmutation(int iPos, DWORD dwTransmutation, bool bSelf)
{
	if (iPos >= EXCHANGE_MAX_NUM)
		return;
	
	if (bSelf)
		m_self.transmutation[iPos] = dwTransmutation;
	else
		m_victim.transmutation[iPos] = dwTransmutation;
}

DWORD CPythonExchange::GetItemTransmutation(int iPos, bool bSelf)
{
	if (iPos >= EXCHANGE_MAX_NUM)
		return 0;
	
	DWORD dwTransmutation = bSelf == true ? m_self.transmutation[iPos] : m_victim.transmutation[iPos];
	return dwTransmutation;
}
#endif

void CPythonExchange::SetItemToTarget(TItemPos pos, DWORD vnum, BYTE count)
{
	if (pos.cell >= EXCHANGE_MAX_NUM)
		return;

	for (int i = 0; i < pos.window_type; ++i)
	{
		int p = pos.cell + (i * EXCHANGE_PAGE_COLUMN);

		if (p >= EXCHANGE_MAX_NUM)
			continue;

		m_victim.item_grid[p] = pos.cell + 1;
	}

	m_victim.item_vnum[pos.cell] = vnum;
	m_victim.item_count[pos.cell] = count;
}

void CPythonExchange::SetItemToSelf(TItemPos pos, DWORD vnum, BYTE count)
{
	if (pos.cell >= EXCHANGE_MAX_NUM)
		return;

	for (int i = 0; i < pos.window_type; ++i)
	{
		int p = pos.cell + (i * EXCHANGE_PAGE_COLUMN);

		if (p >= EXCHANGE_MAX_NUM)
			continue;

		m_self.item_grid[p] = pos.cell + 1;
	}

	m_self.item_vnum[pos.cell] = vnum;
	m_self.item_count[pos.cell] = count;
}

void CPythonExchange::SetItemMetinSocketToTarget(int pos, int imetinpos, DWORD vnum)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return;

	m_victim.item_metin[pos][imetinpos] = vnum;
}

void CPythonExchange::SetItemMetinSocketToSelf(int pos, int imetinpos, DWORD vnum)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return;

	m_self.item_metin[pos][imetinpos] = vnum;
}

void CPythonExchange::SetItemAttributeToTarget(int pos, int iattrpos, BYTE byType, int sValue)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return;

	m_victim.item_attr[pos][iattrpos].bType = byType;
	m_victim.item_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::SetItemAttributeToSelf(int pos, int iattrpos, BYTE byType, int sValue)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return;

	m_self.item_attr[pos][iattrpos].bType = byType;
	m_self.item_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::DelItemOfTarget(BYTE pos, BYTE size)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return;

	for (int i = 0; i < size; ++i)
	{
		int p = pos + (i * EXCHANGE_PAGE_COLUMN);

		if (p >= EXCHANGE_MAX_NUM)
			continue;

		m_victim.item_grid[p] = 0;
	}

	m_victim.item_vnum[pos] = 0;
	m_victim.item_count[pos] = 0;
}

void CPythonExchange::DelItemOfSelf(BYTE pos, BYTE size)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return;

	for (int i = 0; i < size; ++i)
	{
		int p = pos + (i * EXCHANGE_PAGE_COLUMN);

		if (p >= EXCHANGE_MAX_NUM)
			continue;

		m_self.item_grid[p] = 0;
	}

	m_self.item_vnum[pos] = 0;
	m_self.item_count[pos] = 0;
}

DWORD CPythonExchange::GetItemVnumFromTarget(BYTE pos)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return 0;

	return m_victim.item_vnum[pos];
}

DWORD CPythonExchange::GetItemVnumFromSelf(BYTE pos)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return 0;

	return m_self.item_vnum[pos];
}

BYTE CPythonExchange::GetItemCountFromTarget(BYTE pos)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return 0;

	return m_victim.item_count[pos];
}

BYTE CPythonExchange::GetItemCountFromSelf(BYTE pos)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return 0;

	return m_self.item_count[pos];
}

BYTE CPythonExchange::GetItemGridFromTarget(BYTE pos)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return 0;

	return m_victim.item_grid[pos];
}

BYTE CPythonExchange::GetItemGridFromSelf(BYTE pos)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return 0;

	return m_self.item_grid[pos];
}

DWORD CPythonExchange::GetItemMetinSocketFromTarget(BYTE pos, int iMetinSocketPos)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return 0;

	return m_victim.item_metin[pos][iMetinSocketPos];
}

DWORD CPythonExchange::GetItemMetinSocketFromSelf(BYTE pos, int iMetinSocketPos)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return 0;

	return m_self.item_metin[pos][iMetinSocketPos];
}

void CPythonExchange::GetItemAttributeFromTarget(BYTE pos, int iAttrPos, BYTE * pbyType, int * psValue)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return;

	*pbyType = m_victim.item_attr[pos][iAttrPos].bType;
	*psValue = m_victim.item_attr[pos][iAttrPos].sValue;
}

void CPythonExchange::GetItemAttributeFromSelf(BYTE pos, int iAttrPos, BYTE * pbyType, int * psValue)
{
	if (pos >= EXCHANGE_MAX_NUM)
		return;

	*pbyType = m_self.item_attr[pos][iAttrPos].bType;
	*psValue = m_self.item_attr[pos][iAttrPos].sValue;
}

void CPythonExchange::SetAcceptToTarget(BYTE Accept)
{
	m_victim.accept = Accept ? true : false;
}

void CPythonExchange::SetAcceptToSelf(BYTE Accept)
{
	m_self.accept = Accept ? true : false;
}

bool CPythonExchange::GetAcceptFromTarget()
{
	return m_victim.accept ? true : false;
}

bool CPythonExchange::GetAcceptFromSelf()
{
	return m_self.accept ? true : false;
}

bool CPythonExchange::GetElkMode()
{
	return m_elk_mode;
}

void CPythonExchange::SetElkMode(bool value)
{
	m_elk_mode = value;
}

void CPythonExchange::Start()
{
	m_isTrading = true;
}

void CPythonExchange::End()
{
	m_isTrading = false;
}

bool CPythonExchange::isTrading()
{
	return m_isTrading;
}

void CPythonExchange::Clear()
{
	memset(&m_self, 0, sizeof(m_self));
	memset(&m_victim, 0, sizeof(m_victim));
}

CPythonExchange::CPythonExchange()
{
	Clear();
	m_isTrading = false;
	m_elk_mode = false;
}
CPythonExchange::~CPythonExchange()
{
}