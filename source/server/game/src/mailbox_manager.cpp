#include "stdafx.h"
#include "char.h"
#include "mailbox_manager.h"
#include "char_manager.h"
#include "packet.h"
#include "messenger_manager.h"
#include "desc.h"
#include "item.h"
#include "item_manager.h"
#include "buffer_manager.h"
#include "utils.h"
#include "banword.h"
#include "p2p.h"
#include "config.h"
#include "db.h"

#ifdef __MAILBOX__

bool CheckString(const char *str)
{
	const char*	tmp;

	if (!str || !*str)
		return false;

	// if (strlen(str) < 2)
		// return false;

	for (tmp = str; *tmp; ++tmp)
	{
		if (isdigit(*tmp) || isalpha(*tmp))
			continue;

		switch (*tmp)
		{
			case ' ':
			case '_':
			case '-':
			case '.':
			case '!':
			case '@':
			case '#':
			case '$':
			case '%':
			case '^':
			case '&':
			case '*':
			case '(':
			case ')':
				continue;
		}

		return false;
	}

	return true;
}

CMailBoxManager::CMailBoxManager(void)
{
	Initialize();
}

CMailBoxManager::~CMailBoxManager(void)
{
	
}

void CMailBoxManager::Initialize(void)
{

}

void CMailBoxManager::Close(LPCHARACTER ch)
{
	if (!ch || !ch->GetDesc())
		return;

	TPacketGCMailBox pack;
	pack.bHeader = HEADER_GC_MAILBOX;
	pack.bSubHeader = MAILBOX_GC_CLOSE;
	pack.wSize = sizeof(pack);
	ch->GetDesc()->Packet(&pack, sizeof(pack));
}

void CMailBoxManager::WriteConfirm(LPCHARACTER ch, const char * szName)
{
	if (!ch || !ch->GetDesc())
		return;

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(szName);
	BYTE bData = POST_WRITE_FAIL;

	if (tch && tch == ch)
		bData = POST_WRITE_INVALID_NAME;
#ifdef __MESSENGER_BLOCK__
	else if (MessengerManager::instance().CheckMessengerList(ch, szName, SYST_BLOCK))
		bData = POST_WRITE_BLOCKED_ME;
	else if (tch && tch->GetDesc() && MessengerManager::instance().CheckMessengerList(tch, ch->GetName(), SYST_BLOCK))
		bData = POST_WRITE_TARGET_BLOCKED;
#endif
	else if (tch && tch->GetDesc() && tch->FindSpaceMail() == -1)
		bData = POST_WRITE_FULL_MAILBOX;
	else if (!tch || !tch->GetDesc())
	{
		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT mails FROM player.player WHERE name = '%s'", szName));
		if (pMsg->Get()->uiNumRows > 0)
		{
			int space = -1;
			TMailData Mails[MAIL_SLOT_MAX_NUM];
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			if (row[0])
				memcpy(Mails, row[0], sizeof(Mails));
			else
				memset(Mails, 0, sizeof(Mails));

			for (int i = 0; i < MAIL_SLOT_MAX_NUM; ++i)
				if (!Mails[i].send_time)
					space = i;

			if (space == -1)
				bData = POST_WRITE_FULL_MAILBOX;
			else
				bData = POST_WRITE_OK;
		}
		else
			bData = POST_WRITE_INVALID_NAME;
	}
	else
		bData = POST_WRITE_OK;

	TPacketGCMailBox pack;
	pack.bHeader = HEADER_GC_MAILBOX;
	pack.bSubHeader = MAILBOX_GC_POST_WRITE_CONFIRM;

	pack.wSize = sizeof(pack) + sizeof(bData);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
	ch->GetDesc()->Packet(&bData, sizeof(bData));
}

void CMailBoxManager::Write(LPCHARACTER ch, const char * szName, const char * szTitle, const char * szMessage, BYTE bWindowType, DWORD dwItemPos, DWORD dwMoney, DWORD dwCheque)
{
	if (!ch || !ch->GetDesc())
		return;

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(szName);
	CCI * pkCCI = P2P_MANAGER::instance().Find(szName);
	LPDESC pkDesc = NULL;
	BYTE bData = POST_WRITE_FAIL;
	LPITEM item = ch->GetItem(TItemPos(bWindowType, dwItemPos));
	TPacketGCMailBox pack;
	pack.bHeader = HEADER_GC_MAILBOX;

	if (tch && tch == ch)
		bData = POST_WRITE_INVALID_NAME;
#ifdef __MESSENGER_BLOCK__
	if (MessengerManager::instance().CheckMessengerList(ch, szName, SYST_BLOCK))
		bData = POST_WRITE_BLOCKED_ME;
	else if (tch && tch->GetDesc() && MessengerManager::instance().CheckMessengerList(tch, ch->GetName(), SYST_BLOCK))
		bData = POST_WRITE_TARGET_BLOCKED;
#endif
	else if (ch->GetLevel() < 20)
		bData = POST_WRITE_LEVEL_NOT_ENOUGHT;
	else if (tch && tch->GetDesc() && tch->FindSpaceMail() == -1)
		bData = POST_WRITE_FULL_MAILBOX;
	else if (!CheckString(szTitle))
		bData = POST_WRITE_WRONG_TITLE;
	else if (ch->GetGold() < (INT)dwMoney)
		bData = POST_WRITE_YANG_NOT_ENOUGHT;
#ifdef __CHEQUE__
	else if (ch->GetCheque() < (short)dwCheque)
		bData = POST_WRITE_WON_NOT_ENOUGHT;
#endif
	else if (CBanwordManager::instance().CheckString(szMessage, strlen(szMessage)))
		bData = POST_WRITE_WRONG_MESSAGE;
#ifdef __SOULBIND__
	else if (item && item->IsSealed())
		bData = POST_WRITE_WRONG_ITEM;
#endif
	else
	{
		int space = -1;
		TMailData Mails[MAIL_SLOT_MAX_NUM];

		if (tch && tch->GetDesc())
			bData = POST_WRITE_OK;
		else
		{
			std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT id, mails FROM player.player WHERE name = '%s'", szName));
			if (pMsg->Get()->uiNumRows > 0)
			{
				MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
				int id = 0;
				str_to_number(id, row[0]);
				if (row[1])
					memcpy(Mails, row[1], sizeof(Mails));
				else
					memset(Mails, 0, sizeof(Mails));

				for (int i = 0; i < MAIL_SLOT_MAX_NUM; ++i)
					if (!Mails[i].send_time)
					{
						space = i;
						break;
					}

				if (space == -1)
					bData = POST_WRITE_FULL_MAILBOX;
				else
					bData = POST_WRITE_OK;
			}
			else
				bData = POST_WRITE_INVALID_NAME;
		}

		if (bData == POST_WRITE_OK)
		{
			TMailData Mail;
			memset(&Mail, 0, sizeof(Mail));
			Mail.index = tch ? tch->FindSpaceMail() : space;
			strlcpy(Mail.from_name, ch->GetName(), sizeof(Mail.from_name));
			strlcpy(Mail.message, szMessage, sizeof(Mail.message));
			strlcpy(Mail.title, szTitle, sizeof(Mail.title));
			Mail.yang = dwMoney;
#ifdef __CHEQUE__
			Mail.cheque = dwCheque;
#endif
			Mail.send_time = get_global_time();
			Mail.delete_time = get_global_time() + 30 * 24 * 60 * 60;
			Mail.is_gm_post = ch->IsGM();
			Mail.is_confirm = false;
			if (item)
			{
				Mail.is_item_exist = true;
				Mail.item.count = item->GetCount();
				Mail.item.vnum = item->GetVnum();
				Mail.item.flags = item->GetFlag();
				Mail.item.anti_flags = item->GetAntiFlag();

				memcpy(Mail.item.alSockets, item->GetSockets(), sizeof(Mail.item.alSockets));
				memcpy(Mail.item.aAttr, item->GetAttributes(), sizeof(Mail.item.aAttr));
#ifdef __SOULBIND__
				Mail.item.sealbind = item->GetSealBindTime();
#endif
#ifdef __CHANGELOOK__
				Mail.item.transmutation = item->GetTransmutation();
#endif
				Mail.item.bIsBasic = item->IsBasicItem();
			}
			else
			{
				Mail.is_item_exist = false;
				memset(&Mail.item, 0, sizeof(Mail.item));
			}

			ch->PointChange(POINT_GOLD, -dwMoney, true);
			ch->PointChange(POINT_GOLD, -1000, true);
#ifdef __CHEQUE__
			ch->PointChange(POINT_CHEQUE, -dwCheque, true);
#endif
			if (item)
				ITEM_MANAGER::instance().RemoveItem(item);

			if (tch && tch->GetDesc())
			{
				tch->AddMail(Mail.index, Mail);
				pack.bSubHeader = MAILBOX_GC_UNREAD_DATA;
				TEMP_BUFFER buf;
				buf.write(&Mail, sizeof(Mail));
				pack.wSize = sizeof(pack) + buf.size();

				tch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
				tch->GetDesc()->Packet(buf.read_peek(), buf.size());
			}
			else
			{
				if (pkCCI)
				{
					pkDesc = pkCCI->pkDesc;
					pkDesc->SetRelay(szName);
					pack.bSubHeader = MAILBOX_GC_UNREAD_DATA;
					pack.wSize = sizeof(pack) + sizeof(Mail);
					TEMP_BUFFER buf;
					buf.write(&pack, sizeof(pack));
					buf.write(&Mail, sizeof(Mail));

					pkDesc->Packet(buf.read_peek(), buf.size());
					pkDesc->SetRelay("");
				}
				else
				{
					Mails[space] = Mail;
					static char text[QUERY_MAX_LEN + 1];
					DBManager::instance().EscapeString((char *)text, sizeof(text), (const char *)Mails, sizeof(Mails));
					DBManager::instance().DirectQuery("UPDATE player.player SET mails = '%s' WHERE name = '%s'", text, szName);
				}
			}
		}
	}

	pack.bSubHeader = MAILBOX_GC_POST_WRITE;
	pack.wSize = sizeof(pack) + sizeof(bData);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
	ch->GetDesc()->Packet(&bData, sizeof(bData));
}

void CMailBoxManager::PostGetItems(LPCHARACTER ch, BYTE bDataIndex)
{
	if (!ch || !ch->GetDesc())
		return;

	BYTE bData = POST_GET_ITEMS_FAIL;
	TMailData * Mail = ch->GetMail(bDataIndex);

	if (ch->GetGold() + Mail->yang >= GOLD_MAX)
		bData = POST_GET_ITEMS_YANG_OVERFLOW;
#ifdef __CHEQUE__
	else if (ch->GetCheque() + Mail->cheque >= GOLD_MAX)
		bData = POST_GET_ITEMS_WON_OVERFLOW;
#endif
#ifdef __MESSENGER_BLOCK__
	else if (MessengerManager::instance().CheckMessengerList(ch, Mail->from_name, SYST_BLOCK))
		bData = POST_GET_ITEMS_FAIL_BLOCK_CHAR;
#endif
	else
	{
		if (Mail->is_item_exist)
		{
			LPITEM item = ITEM_MANAGER::instance().CreateItem(Mail->item.vnum, Mail->item.count);
			if (item)
			{
				int pos;
				if (item->IsDragonSoul())
					pos = ch->GetEmptyDragonSoulInventory(item);
#ifdef __NEW_STORAGE__
				else if (item->IsStorageItem())
					pos = ch->GetEmptyStorageInventory(item->GetStorageType() - UPGRADE_INVENTORY, item);
#endif
				else
					pos = ch->GetEmptyInventory(item->GetSize());

				if (pos == -1)
				{
					bData = POST_GET_ITEMS_NOT_ENOUGHT_INVENTORY;
					ITEM_MANAGER::instance().RemoveItem(item);
				}
				else
				{
					bData = POST_GET_ITEMS_OK;
					item->SetSkipSave(true);
					item->SetFlag(Mail->item.flags);
					item->SetSockets(Mail->item.alSockets);
					item->SetAttributes(Mail->item.aAttr);
#ifdef __CHANGELOOK__
					item->SetTransmutation(Mail->item.transmutation);
#endif
#ifdef __SOULBIND__
					item->SetSealBind(Mail->item.sealbind);
#endif
					item->SetBasic(Mail->item.bIsBasic);
					item->SetSkipSave(false);

					if (item->IsDragonSoul())
						item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, pos));
#ifdef __NEW_STORAGE__
					else if (item->IsStorageItem())
						item->AddToCharacter(ch, TItemPos(UPGRADE_INVENTORY + (item->GetStorageType() - UPGRADE_INVENTORY), pos));
#endif
					else
						item->AddToCharacter(ch, TItemPos(INVENTORY, pos));
					
					ITEM_MANAGER::instance().FlushDelayedSave(item);
					memset(&Mail->item, 0, sizeof(Mail->item));
				}
			}
			else
				bData = POST_GET_ITEMS_NONE;
		}

		if (bData == POST_GET_ITEMS_OK)
		{
			ch->PointChange(POINT_GOLD, Mail->yang, false);
#ifdef __CHEQUE__
			ch->PointChange(POINT_CHEQUE, Mail->cheque, false);
#endif
			Mail->is_item_exist = false;
			Mail->is_confirm = true;
			Mail->yang = 0;
#ifdef __CHEQUE__
			Mail->cheque = 0;
#endif
		}
	}

	TPacketGCMailBox pack;
	pack.bHeader = HEADER_GC_MAILBOX;
	pack.bSubHeader = MAILBOX_GC_POST_GET_ITEMS;

	pack.wSize = sizeof(pack) + sizeof(bData) + sizeof(bDataIndex);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
	ch->GetDesc()->BufferedPacket(&bData, sizeof(bData));
	ch->GetDesc()->Packet(&bDataIndex, sizeof(bDataIndex));
}

void CMailBoxManager::PostAddData(LPCHARACTER ch, BYTE bButtonIndex, BYTE bDataIndex)
{
	if (!ch || !ch->GetDesc())
		return;

	TMailData * Mail = ch->GetMail(bDataIndex);
	Mail->is_confirm = true;

	TPacketGCMailBox pack;
	pack.bHeader = HEADER_GC_MAILBOX;
	pack.bSubHeader = MAILBOX_GC_ADD_DATA;

	pack.wSize = sizeof(pack) + sizeof(bButtonIndex) + sizeof(*Mail);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
	ch->GetDesc()->BufferedPacket(&bButtonIndex, sizeof(bButtonIndex));
	ch->GetDesc()->Packet(Mail, sizeof(*Mail));
}

void CMailBoxManager::PostDelete(LPCHARACTER ch, BYTE bDataIndex)
{
	if (!ch || !ch->GetDesc())
		return;

	BYTE bData = POST_DELETE_FAIL;
	TMailData * Mail = ch->GetMail(bDataIndex);
	if (Mail->is_item_exist)
		bData = POST_DELETE_FAIL_EXIST_ITEMS;
	else
	{
		bData = POST_DELETE_OK;
		ch->DeleteMail(bDataIndex);
	}

	TPacketGCMailBox pack;
	pack.bHeader = HEADER_GC_MAILBOX;
	pack.bSubHeader = MAILBOX_GC_POST_DELETE;

	pack.wSize = sizeof(pack) + sizeof(bData) + sizeof(bDataIndex);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
	ch->GetDesc()->BufferedPacket(&bData, sizeof(bData));
	ch->GetDesc()->Packet(&bDataIndex, sizeof(bDataIndex));
}

void CMailBoxManager::PostAllDelete(LPCHARACTER ch)
{
	if (!ch || !ch->GetDesc())
		return;

	BYTE bData = POST_ALL_DELETE_FAIL;

	for (int i = 0; i < MAIL_SLOT_MAX_NUM; ++i)
	{
		TMailData * Mail = ch->GetMail(i);
		if (!Mail->is_item_exist)
			ch->DeleteMail(i);
	}

	bData = POST_ALL_DELETE_OK;

	TPacketGCMailBox pack;
	pack.bHeader = HEADER_GC_MAILBOX;
	pack.bSubHeader = MAILBOX_GC_POST_ALL_DELETE;

	pack.wSize = sizeof(pack) + sizeof(bData);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
	ch->GetDesc()->Packet(&bData, sizeof(bData));
}

void CMailBoxManager::PostAllGetItems(LPCHARACTER ch)
{
	if (!ch || !ch->GetDesc())
		return;

	BYTE bData = POST_ALL_GET_ITEMS_FAIL;
	BYTE bSuccesList[MAIL_SLOT_MAX_NUM];
	memset(&bSuccesList, 0, sizeof(bSuccesList));

	for (int i = 0; i < MAIL_SLOT_MAX_NUM; ++i)
	{
		TMailData * Mail = ch->GetMail(i);
		if (ch->GetGold() + Mail->yang < GOLD_MAX && ch->GetCheque() + Mail->cheque < GOLD_MAX)
		{
			if (Mail->is_item_exist)
			{
				LPITEM item = ITEM_MANAGER::instance().CreateItem(Mail->item.vnum, Mail->item.count);
				if (item)
				{
					int pos;
					if (item->IsDragonSoul())
						pos = ch->GetEmptyDragonSoulInventory(item);
#ifdef __NEW_STORAGE__
					else if (item->IsStorageItem())
						pos = ch->GetEmptyStorageInventory(item->GetStorageType() - UPGRADE_INVENTORY, item);
#endif
					else
						pos = ch->GetEmptyInventory(item->GetSize());

					if (pos == -1)
					{
						ITEM_MANAGER::instance().RemoveItem(item);
						continue;
					}
					else
					{
						item->SetSkipSave(true);
						item->SetFlag(Mail->item.flags);
						item->SetSockets(Mail->item.alSockets);
						item->SetAttributes(Mail->item.aAttr);
#ifdef __CHANGELOOK__
						item->SetTransmutation(Mail->item.transmutation);
#endif
#ifdef __SOULBIND__
						item->SetSealBind(Mail->item.sealbind);
#endif
						item->SetBasic(Mail->item.bIsBasic);
						item->SetSkipSave(false);

						if (item->IsDragonSoul())
							item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, pos));
#ifdef __NEW_STORAGE__
						else if (item->IsStorageItem())
							item->AddToCharacter(ch, TItemPos(UPGRADE_INVENTORY + (item->GetStorageType() - UPGRADE_INVENTORY), pos));
#endif
						else
							item->AddToCharacter(ch, TItemPos(INVENTORY, pos));
						
						ITEM_MANAGER::instance().FlushDelayedSave(item);
						memset(&Mail->item, 0, sizeof(Mail->item));
					}
				}
				else
					continue;

				ch->PointChange(POINT_GOLD, Mail->yang, false);
#ifdef __CHEQUE__
				ch->PointChange(POINT_CHEQUE, Mail->cheque, false);
#endif
				Mail->is_item_exist = false;
				Mail->is_confirm = true;
				Mail->yang = 0;
#ifdef __CHEQUE__
				Mail->cheque = 0;
#endif
				bSuccesList[i] = 1;
			}
		}
	}

	bData = POST_ALL_GET_ITEMS_OK;

	TPacketGCMailBox pack;
	pack.bHeader = HEADER_GC_MAILBOX;
	pack.bSubHeader = MAILBOX_GC_POST_ALL_GET_ITEMS;

	pack.wSize = sizeof(pack) + sizeof(bData);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
	ch->GetDesc()->BufferedPacket(&bData, sizeof(bData));
	ch->GetDesc()->Packet(&bSuccesList, sizeof(bSuccesList));
}

void CMailBoxManager::CheckMails(LPCHARACTER ch)
{
	if (!ch || !ch->GetDesc())
		return;

	TPacketGCMailBox pack;
	pack.bHeader = HEADER_GC_MAILBOX;
	pack.bSubHeader = MAILBOX_GC_SET;
	TEMP_BUFFER buf;

	for (BYTE i = 0; i < MAIL_SLOT_MAX_NUM; ++i)
	{
		TMailData * Mail = ch->GetMail(i);
		if (Mail->delete_time <= get_global_time())
			ch->DeleteMail(i);
		else
			buf.write(Mail, sizeof(*Mail));
	}

	pack.wSize = sizeof(pack) + buf.size();

	if (buf.size())
	{
		ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
		ch->GetDesc()->Packet(buf.read_peek(), buf.size());
	}
	else
		ch->GetDesc()->Packet(&pack, sizeof(pack));
}

#endif
