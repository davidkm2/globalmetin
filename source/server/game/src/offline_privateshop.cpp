#include "stdafx.h"
#include "offline_privateshop.h"
#include "char_manager.h"
#include "shop.h"
#include "desc.h"

COfflinePrivateShop::COfflinePrivateShop(DWORD dwRaceVnum, DWORD dwTime, LPSHOP pkShop) : m_dwRaceVnum(dwRaceVnum), m_dwVid(CHARACTER_MANAGER::instance().AllocVID()), m_dwEndTime(dwTime), m_pkShop(pkShop)
{
	Initialize();
}

COfflinePrivateShop::~COfflinePrivateShop(void)
{
	Destroy();
}

void COfflinePrivateShop::Initialize(void)
{
	CEntity::Initialize(ENTITY_OFFLINE_PRIVATE_SHOP);
	m_dwShopVid = 0;
	m_dwPlayerID = 0;
	m_strSign = "";
	m_strOwnerName = "";
	m_bDeco = 0;
	memset(&m_dwPrices, 0, sizeof(m_dwPrices));
	m_dwStartTime = time(0);
}

void COfflinePrivateShop::Destroy(void)
{
	CEntity::Destroy();

	if (GetSectree())
		GetSectree()->RemoveEntity(this);

	m_dwRaceVnum = 0;
	m_dwVid = 0;
	m_dwStartTime = 0;
	m_dwEndTime = 0;
	m_dwShopVid = 0;
	m_dwPlayerID = 0;
	M2_DELETE(m_pkShop);
	m_pkShop = NULL;
	m_strSign = "";
	m_strOwnerName = "";
	m_bDeco = 0;
	memset(&m_dwPrices, 0, sizeof(m_dwPrices));
}

void COfflinePrivateShop::EncodeInsertPacket(LPENTITY ent)
{
	if (ent->GetType() != ENTITY_CHARACTER)
		return;

	LPDESC d;

	if (!(d = ent->GetDesc()))
		return;

	TPacketGCCharacterAdd pack;
	memset(&pack, 0, sizeof(pack));

	pack.header = HEADER_GC_CHARACTER_ADD;
	pack.dwVID = m_dwVid;
	pack.bType = CHAR_TYPE_NPC;
	pack.x = GetX();
	pack.y = GetY();
	pack.z = GetZ();
	pack.wRaceNum = m_dwRaceVnum;
	pack.bStateFlag = ADD_CHARACTER_STATE_GUNGON;

	d->Packet(&pack, sizeof(pack));

	TPacketGCCharacterAdditionalInfo addPacket;
	memset(&addPacket, 0, sizeof(TPacketGCCharacterAdditionalInfo));

	addPacket.header = HEADER_GC_CHAR_ADDITIONAL_INFO;
	addPacket.dwVID = m_dwVid;
	strlcpy(addPacket.name, m_strOwnerName.c_str(), sizeof(addPacket.name));

	d->Packet(&addPacket, sizeof(addPacket));

	TPacketGCShopSign p;

	p.bHeader = HEADER_GC_SHOP_SIGN;
	p.dwVID = m_dwVid;
#ifdef __MYSHOP_DECO__
	p.Deco.bDeco = m_bDeco;
#endif
	strlcpy(p.szSign, m_strSign.c_str(), sizeof(p.szSign));

	d->Packet(&p, sizeof(p));
}

void COfflinePrivateShop::EncodeRemovePacket(LPENTITY ent)
{
	if (ent->GetType() != ENTITY_CHARACTER)
		return;

	LPDESC d;

	if (!(d = ent->GetDesc()))
		return;

	TPacketGCCharacterDelete pack;

	pack.header	= HEADER_GC_CHARACTER_DEL;
	pack.id	= m_dwVid;

	d->Packet(&pack, sizeof(TPacketGCCharacterDelete));

	TPacketGCShopSign p;

	p.bHeader = HEADER_GC_SHOP_SIGN;
	p.dwVID = m_dwVid;
	p.szSign[0] = '\0';

	d->Packet(&p, sizeof(p));
}

const DWORD* COfflinePrivateShop::AddPrices(DWORD dwPrice[])
{
	for (int i = 0; i < SHOP_PRICE_MAX_NUM; ++i)
		m_dwPrices[i] += dwPrice[i];

	return &m_dwPrices[0];
}

void COfflinePrivateShop::GetInfo(TOfflinePrivateShopInfo * pkTOfflinePrivateShopInfo)
{
	if (m_pkShop)
		m_pkShop->SetOfflinePrivateShopInfo(pkTOfflinePrivateShopInfo);

	pkTOfflinePrivateShopInfo->dwVid = m_dwVid;
	strlcpy(pkTOfflinePrivateShopInfo->szSign, m_strSign.c_str(), sizeof(pkTOfflinePrivateShopInfo->szSign));
	memcpy(pkTOfflinePrivateShopInfo->dwPrices, m_dwPrices, sizeof(pkTOfflinePrivateShopInfo->dwPrices));
	pkTOfflinePrivateShopInfo->dwX = GetX();
	pkTOfflinePrivateShopInfo->dwY = GetY();
	// pkTOfflinePrivateShopInfo->dwMapIndex = GetMapIndex();
	pkTOfflinePrivateShopInfo->dwStartTime = m_dwStartTime;
	pkTOfflinePrivateShopInfo->dwEndTime = m_dwEndTime;
}

