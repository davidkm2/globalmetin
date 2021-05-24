#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "NetworkActorManager.h"
#include "PythonBackground.h"
#include "AbstractApplication.h"
#include "PythonApplication.h"
#include "AbstractPlayer.h"
#include "../GameLib/ActorInstance.h"

void CPythonNetworkStream::__GlobalPositionToLocalPosition(LONG& rGlobalX, LONG& rGlobalY)
{
	CPythonBackground&rkBgMgr=CPythonBackground::Instance();
	rkBgMgr.GlobalPositionToLocalPosition(rGlobalX, rGlobalY);
}

void CPythonNetworkStream::__LocalPositionToGlobalPosition(LONG& rLocalX, LONG& rLocalY)
{
	CPythonBackground&rkBgMgr=CPythonBackground::Instance();
	rkBgMgr.LocalPositionToGlobalPosition(rLocalX, rLocalY);
}

bool CPythonNetworkStream::__CanActMainInstance()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return false;

	return pkInstMain->CanAct();
}

void CPythonNetworkStream::__ClearNetworkActorManager()
{
	m_rokNetActorMgr->Destroy();
}

void __SetWeaponPower(IAbstractPlayer& rkPlayer, DWORD dwWeaponID)
{
	DWORD minPower=0;
	DWORD maxPower=0;
	DWORD minMagicPower=0;
	DWORD maxMagicPower=0;
	DWORD addPower=0;

	CItemData* pkWeapon;
#if defined(ENABLE_WEAPON_COSTUME_SYSTEM) || defined(ENABLE_CHANGE_LOOK_SYSTEM)
	if (CItemManager::Instance().GetItemDataPointer(CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, EQUIPMENT_WEAPON)), &pkWeapon))
#else
	if (CItemManager::Instance().GetItemDataPointer(dwWeaponID, &pkWeapon))
#endif
	{
		if (pkWeapon->GetType()==CItemData::ITEM_TYPE_WEAPON)
		{
			minPower=pkWeapon->GetValue(3);
			maxPower=pkWeapon->GetValue(4);
			minMagicPower=pkWeapon->GetValue(1);
			maxMagicPower=pkWeapon->GetValue(2);
			addPower=pkWeapon->GetValue(5);
		}
	}

	rkPlayer.SetWeaponPower(minPower, maxPower, minMagicPower, maxMagicPower, addPower);

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	CItemData * pItemData;
	if (CItemManager::Instance().GetItemDataPointer(CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, EQUIPMENT_WEAPON)), &pItemData))
	{
		minPower=pItemData->GetValue(3);
		maxPower=pItemData->GetValue(4);
		minMagicPower=pItemData->GetValue(1);
		maxMagicPower=pItemData->GetValue(2);
		addPower=pItemData->GetValue(5);
	}
#endif	

}
bool IsInvisibleRace(WORD raceNum)
{
	switch(raceNum)
	{
	case 20025:
	case 20038:
	case 20039:
		return true;
	default:
		return false;
	}
}

static SNetworkActorData s_kNetActorData;
bool CPythonNetworkStream::RecvCharacterAppendPacket()
{
	TPacketGCCharacterAdd chrAddPacket;
	if (!Recv(sizeof(chrAddPacket), &chrAddPacket))
		return false;

	__GlobalPositionToLocalPosition(chrAddPacket.x, chrAddPacket.y);

	SNetworkActorData kNetActorData;
	kNetActorData.m_bType=chrAddPacket.bType;
	kNetActorData.m_dwMovSpd=chrAddPacket.bMovingSpeed;
	kNetActorData.m_dwAtkSpd=chrAddPacket.bAttackSpeed;
	kNetActorData.m_dwRace=chrAddPacket.wRaceNum;

	kNetActorData.m_dwStateFlags=chrAddPacket.bStateFlag;
	kNetActorData.m_dwVID=chrAddPacket.dwVID;
	kNetActorData.m_fRot=chrAddPacket.angle;

	if (kNetActorData.m_bType == CActorInstance::TYPE_NPC)
	{
		const char * c_szName;
		CPythonNonPlayer & rkNonPlayer = CPythonNonPlayer::Instance();
		if (rkNonPlayer.GetName(kNetActorData.m_dwRace, &c_szName))
			kNetActorData.m_stName = c_szName;
		else
			kNetActorData.m_stName = "";
	}
	else
		kNetActorData.m_stName = "";

	kNetActorData.m_kAffectFlags.CopyData(0, sizeof(chrAddPacket.dwAffectFlag[0]), &chrAddPacket.dwAffectFlag[0]);
	kNetActorData.m_kAffectFlags.CopyData(32, sizeof(chrAddPacket.dwAffectFlag[1]), &chrAddPacket.dwAffectFlag[1]);

	kNetActorData.SetPosition(chrAddPacket.x, chrAddPacket.y);

	kNetActorData.m_sAlignment=0;/*chrAddPacket.sAlignment*/;
	kNetActorData.m_byPKMode=0;/*chrAddPacket.bPKMode*/;
	kNetActorData.m_dwGuildID=0;/*chrAddPacket.dwGuild*/;
	kNetActorData.m_dwEmpireID=0;/*chrAddPacket.bEmpire*/;
	kNetActorData.m_dwArmor=0;/*chrAddPacket.awPart[CHR_EQUIPPART_ARMOR]*/;
	kNetActorData.m_dwWeapon=0;/*chrAddPacket.awPart[CHR_EQUIPPART_WEAPON]*/;
	kNetActorData.m_dwHair=0;/*chrAddPacket.awPart[CHR_EQUIPPART_HAIR]*/;
#ifdef ENABLE_ACCE_SYSTEM
	kNetActorData.m_dwAcce=0;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	kNetActorData.m_dwArrowType=0;
#endif
#ifdef ENABLE_COSTUME_EFFECT
	kNetActorData.m_dwBodyEffect=0;
	kNetActorData.m_dwWeaponEffectRight=0;
	kNetActorData.m_dwWeaponEffectLeft=0;
#endif

	kNetActorData.m_dwMountVnum=0;/*chrAddPacket.dwMountVnum*/;
#ifdef ENABLE_GUILD_GENERAL_AND_LEADER
	kNetActorData.m_dwNewIsGuildName=0;
#endif
#ifdef ENABLE_BATTLE_FIELD
	kNetActorData.m_bBattleFieldRank=0;
#endif
#ifdef ENABLE_ELEMENT_ADD
	kNetActorData.m_bElement=0;
#endif
#ifdef ENABLE_MULTI_LANGUAGE
	kNetActorData.m_stLanguage = "";
#endif
	kNetActorData.m_dwLevel = chrAddPacket.dwLevel;

	if(kNetActorData.m_bType != CActorInstance::TYPE_PC &&
		kNetActorData.m_bType != CActorInstance::TYPE_NPC &&
		kNetActorData.m_bType != CActorInstance::TYPE_PET
#ifdef ENABLE_GROWTH_PET_SYSTEM
&& kNetActorData.m_bType != CActorInstance::TYPE_NEW_PET
#endif
)
	{
		const char * c_szName;
		CPythonNonPlayer& rkNonPlayer=CPythonNonPlayer::Instance();
		if (rkNonPlayer.GetName(kNetActorData.m_dwRace, &c_szName))
			kNetActorData.m_stName = c_szName;
		__RecvCharacterAppendPacket(&kNetActorData);
	}
	else
		s_kNetActorData = kNetActorData;

	return true;
}

bool CPythonNetworkStream::RecvCharacterAdditionalInfo()
{
	TPacketGCCharacterAdditionalInfo chrInfoPacket;
	if (!Recv(sizeof(chrInfoPacket), &chrInfoPacket))
		return false;
	SNetworkActorData kNetActorData = s_kNetActorData;
	if (IsInvisibleRace(kNetActorData.m_dwRace))
		return true;

	if (kNetActorData.m_dwVID == chrInfoPacket.dwVID)
	{
		if (kNetActorData.m_bType != CActorInstance::TYPE_NPC || (kNetActorData.m_dwRace >= 30000
#ifdef ENABLE_MYSHOP_DECO
 && kNetActorData.m_dwRace <= 30007
#endif
))
		{
#ifdef ENABLE_SUPPORT_SYSTEM
			if (kNetActorData.m_dwRace == 34001)
			{
				IAbstractApplication & rkApp = IAbstractApplication::GetSingleton();
				string str(chrInfoPacket.name);
				str += " - " + rkApp.FindLocaleQuest(2544);
				kNetActorData.m_stName = str.c_str();
			}
			else
				kNetActorData.m_stName = chrInfoPacket.name;
#else
			kNetActorData.m_stName = chrInfoPacket.name;
#endif
		}
		kNetActorData.m_dwGuildID = chrInfoPacket.dwGuildID;
		kNetActorData.m_dwLevel = chrInfoPacket.dwLevel;
		kNetActorData.m_sAlignment=chrInfoPacket.sAlignment;
		kNetActorData.m_byPKMode=chrInfoPacket.bPKMode;
		kNetActorData.m_dwGuildID=chrInfoPacket.dwGuildID;
		kNetActorData.m_dwEmpireID=chrInfoPacket.bEmpire;
		kNetActorData.m_dwArmor=chrInfoPacket.awPart[CHR_EQUIPPART_ARMOR];
		kNetActorData.m_dwWeapon=chrInfoPacket.awPart[CHR_EQUIPPART_WEAPON];
		kNetActorData.m_dwHair=chrInfoPacket.awPart[CHR_EQUIPPART_HAIR];
#ifdef ENABLE_ACCE_SYSTEM
		kNetActorData.m_dwAcce = chrInfoPacket.awPart[CHR_EQUIPPART_ACCE];
#endif
#ifdef ENABLE_QUIVER_SYSTEM
		kNetActorData.m_dwArrowType=chrInfoPacket.awPart[CHR_EQUIPPART_ARROW_TYPE];
#endif
		kNetActorData.m_dwMountVnum=chrInfoPacket.dwMountVnum;
#ifdef ENABLE_GUILD_GENERAL_AND_LEADER
		kNetActorData.m_dwNewIsGuildName=chrInfoPacket.dwNewIsGuildName;
#endif
#ifdef ENABLE_COSTUME_EFFECT
		kNetActorData.m_dwBodyEffect=chrInfoPacket.awPart[CHR_EQUIPPART_BODY_EFFECT];
		kNetActorData.m_dwWeaponEffectRight=chrInfoPacket.awPart[CHR_EQUIPPART_WEAPON_RIGHT_EFFECT];
		kNetActorData.m_dwWeaponEffectLeft=chrInfoPacket.awPart[CHR_EQUIPPART_WEAPON_LEFT_EFFECT];
#endif
#ifdef ENABLE_BATTLE_FIELD
		kNetActorData.m_bBattleFieldRank = chrInfoPacket.bBattleFieldRank;
#endif
#ifdef ENABLE_ELEMENT_ADD
		kNetActorData.m_bElement = chrInfoPacket.bElement;
#endif
#ifdef ENABLE_MULTI_LANGUAGE
		kNetActorData.m_stLanguage = chrInfoPacket.szLanguage;
#endif
		__RecvCharacterAppendPacket(&kNetActorData);
	}
	else
	{
		TraceError("TPacketGCCharacterAdditionalInfo name=%s vid=%d mvid=%d race=%d Error", chrInfoPacket.name, chrInfoPacket.dwVID, kNetActorData.m_dwVID, kNetActorData.m_dwRace);
	}
	return true;
}

bool CPythonNetworkStream::RecvCharacterUpdatePacket()
{
	TPacketGCCharacterUpdate chrUpdatePacket;
	if (!Recv(sizeof(chrUpdatePacket), &chrUpdatePacket))
		return false;

	SNetworkUpdateActorData kNetUpdateActorData;
	kNetUpdateActorData.m_dwGuildID=chrUpdatePacket.dwGuildID;
	kNetUpdateActorData.m_dwMovSpd=chrUpdatePacket.bMovingSpeed;
	kNetUpdateActorData.m_dwAtkSpd=chrUpdatePacket.bAttackSpeed;
	kNetUpdateActorData.m_dwArmor=chrUpdatePacket.awPart[CHR_EQUIPPART_ARMOR];
	kNetUpdateActorData.m_dwWeapon=chrUpdatePacket.awPart[CHR_EQUIPPART_WEAPON];
	kNetUpdateActorData.m_dwHair=chrUpdatePacket.awPart[CHR_EQUIPPART_HAIR];
#ifdef ENABLE_ACCE_SYSTEM
	kNetUpdateActorData.m_dwAcce=chrUpdatePacket.awPart[CHR_EQUIPPART_ACCE];
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	kNetUpdateActorData.m_dwArrowType=chrUpdatePacket.awPart[CHR_EQUIPPART_ARROW_TYPE];
#endif
	kNetUpdateActorData.m_dwVID=chrUpdatePacket.dwVID;
	kNetUpdateActorData.m_kAffectFlags.CopyData(0, sizeof(chrUpdatePacket.dwAffectFlag[0]), &chrUpdatePacket.dwAffectFlag[0]);
	kNetUpdateActorData.m_kAffectFlags.CopyData(32, sizeof(chrUpdatePacket.dwAffectFlag[1]), &chrUpdatePacket.dwAffectFlag[1]);
	kNetUpdateActorData.m_sAlignment=chrUpdatePacket.sAlignment;
	kNetUpdateActorData.m_dwLevel=chrUpdatePacket.dwLevel;
	kNetUpdateActorData.m_byPKMode=chrUpdatePacket.bPKMode;
	kNetUpdateActorData.m_dwStateFlags=chrUpdatePacket.bStateFlag;
	kNetUpdateActorData.m_dwMountVnum=chrUpdatePacket.dwMountVnum;
#ifdef ENABLE_GUILD_GENERAL_AND_LEADER
	kNetUpdateActorData.m_dwNewIsGuildName=chrUpdatePacket.dwNewIsGuildName;
#endif
#ifdef ENABLE_EXTEND_INVEN_SYSTEM
	IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
	if (chrUpdatePacket.bStage || !rPlayer.GetExtendInvenStage())
		rPlayer.SetExtendInvenStage(chrUpdatePacket.bStage);
		
#endif
#ifdef ENABLE_COSTUME_EFFECT
	kNetUpdateActorData.m_dwBodyEffect=chrUpdatePacket.awPart[CHR_EQUIPPART_BODY_EFFECT];
	kNetUpdateActorData.m_dwWeaponEffectRight=chrUpdatePacket.awPart[CHR_EQUIPPART_WEAPON_RIGHT_EFFECT];
	kNetUpdateActorData.m_dwWeaponEffectLeft=chrUpdatePacket.awPart[CHR_EQUIPPART_WEAPON_LEFT_EFFECT];
#endif
#ifdef ENABLE_BATTLE_FIELD
	kNetUpdateActorData.m_bBattleFieldRank = chrUpdatePacket.bBattleFieldRank;
#endif
#ifdef ENABLE_ELEMENT_ADD
	kNetUpdateActorData.m_bElement = chrUpdatePacket.bElement;
#endif
	__RecvCharacterUpdatePacket(&kNetUpdateActorData);

	return true;
}

void CPythonNetworkStream::__RecvCharacterAppendPacket(SNetworkActorData * pkNetActorData)
{
	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	if (rkPlayer.IsMainCharacterIndex(pkNetActorData->m_dwVID))
	{
		rkPlayer.SetRace(pkNetActorData->m_dwRace);

		__SetWeaponPower(rkPlayer, pkNetActorData->m_dwWeapon);

		if (rkPlayer.NEW_GetMainActorPtr())
		{
			CPythonBackground::Instance().Update(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY, 0.0f);
			CPythonCharacterManager::Instance().Update();

			{
				std::string strMapName = CPythonBackground::Instance().GetWarpMapName();
				if (strMapName == "metin2_map_deviltower1")
					__ShowMapName(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY);
			}
		}
		else
		{
			__ShowMapName(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY);
		}
	}

	m_rokNetActorMgr->AppendActor(*pkNetActorData);

	if (GetMainActorVID()==pkNetActorData->m_dwVID)
	{
		rkPlayer.SetTarget(0);
		if (m_bComboSkillFlag)
			rkPlayer.SetComboSkillFlag(m_bComboSkillFlag);

		__SetGuildID(pkNetActorData->m_dwGuildID);

	}
}

void CPythonNetworkStream::__RecvCharacterUpdatePacket(SNetworkUpdateActorData * pkNetUpdateActorData)
{
	m_rokNetActorMgr->UpdateActor(*pkNetUpdateActorData);

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	if (rkPlayer.IsMainCharacterIndex(pkNetUpdateActorData->m_dwVID))
	{
		__SetGuildID(pkNetUpdateActorData->m_dwGuildID);
		__SetWeaponPower(rkPlayer, pkNetUpdateActorData->m_dwWeapon);

		__RefreshStatus();
		__RefreshAlignmentWindow();
		__RefreshEquipmentWindow();
		__RefreshInventoryWindow();
	}
	else
	{
		rkPlayer.NotifyCharacterUpdate(pkNetUpdateActorData->m_dwVID);
	}
}

bool CPythonNetworkStream::RecvCharacterDeletePacket()
{
	TPacketGCCharacterDelete chrDelPacket;

	if (!Recv(sizeof(chrDelPacket), &chrDelPacket))
	{
		TraceError("CPythonNetworkStream::RecvCharacterDeletePacket - Recv Error");
		return false;
	}

	m_rokNetActorMgr->RemoveActor(chrDelPacket.dwVID);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
		"BINARY_PrivateShop_Disappear",
		Py_BuildValue("(i)", chrDelPacket.dwVID)
	);

	return true;
}
bool CPythonNetworkStream::RecvCharacterMovePacket()
{
	TPacketGCMove kMovePacket;
	if (!Recv(sizeof(TPacketGCMove), &kMovePacket))
	{
		Tracen("CPythonNetworkStream::RecvCharacterMovePacket - PACKET READ ERROR");
		return false;
	}

	__GlobalPositionToLocalPosition(kMovePacket.lX, kMovePacket.lY);

	SNetworkMoveActorData kNetMoveActorData;
	kNetMoveActorData.m_dwArg=kMovePacket.bArg;
	kNetMoveActorData.m_dwFunc=kMovePacket.bFunc;
	kNetMoveActorData.m_dwTime=kMovePacket.dwTime;
	kNetMoveActorData.m_dwVID=kMovePacket.dwVID;
	kNetMoveActorData.m_fRot=kMovePacket.bRot*5.0f;
	kNetMoveActorData.m_lPosX=kMovePacket.lX;
	kNetMoveActorData.m_lPosY=kMovePacket.lY;
	kNetMoveActorData.m_dwDuration=kMovePacket.dwDuration;

	m_rokNetActorMgr->MoveActor(kNetMoveActorData);

	return true;
}

bool CPythonNetworkStream::RecvOwnerShipPacket()
{
	TPacketGCOwnership kPacketOwnership;

	if (!Recv(sizeof(kPacketOwnership), &kPacketOwnership))
		return false;

	m_rokNetActorMgr->SetActorOwner(kPacketOwnership.dwOwnerVID, kPacketOwnership.dwVictimVID);

	return true;
}

bool CPythonNetworkStream::RecvSyncPositionPacket()
{
	TPacketGCSyncPosition kPacketSyncPos;
	if (!Recv(sizeof(kPacketSyncPos), &kPacketSyncPos))
		return false;

	TPacketGCSyncPositionElement kSyncPos;

	UINT uSyncPosCount=(kPacketSyncPos.wSize-sizeof(kPacketSyncPos))/sizeof(kSyncPos);
	for (UINT iSyncPos=0; iSyncPos<uSyncPosCount; ++iSyncPos)
	{
		if (!Recv(sizeof(TPacketGCSyncPositionElement), &kSyncPos))
			return false;

#ifdef __MOVIE_MODE__
		return true;
#endif __MOVIE_MODE__
		__GlobalPositionToLocalPosition(kSyncPos.lX, kSyncPos.lY);
		m_rokNetActorMgr->SyncActor(kSyncPos.dwVID, kSyncPos.lX, kSyncPos.lY);
}

	return true;
}
