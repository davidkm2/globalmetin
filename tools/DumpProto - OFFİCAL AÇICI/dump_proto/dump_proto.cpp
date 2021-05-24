#include "dump_proto.h"
#include <locale.h>

bool LoadNPrint = false;

CPythonNonPlayer::TMobTable * m_pMobTable = NULL;
int m_iMobTableSize = 0;

CItemData::TItemTable * m_pItemTable = NULL;
int m_iItemTableSize = 0;


bool Set_Proto_Mob_Table(CPythonNonPlayer::TMobTable *mobTable, cCsvTable &csvTable, std::map<int,const char*> &nameMap)
{
	int col = 0;

	mobTable->dwVnum               = atoi(csvTable.AsStringByIndex(col++));
	strncpy(mobTable->szName, csvTable.AsStringByIndex(col++), CHARACTER_NAME_MAX_LEN);
	map<int,const char*>::iterator it;
	it = nameMap.find(mobTable->dwVnum);
	if (it != nameMap.end())
	{
		const char * localeName = it->second;
		strncpy(mobTable->szLocaleName, localeName, sizeof (mobTable->szLocaleName));
	}
	else
		strncpy(mobTable->szLocaleName, mobTable->szName, sizeof (mobTable->szLocaleName));

	int rankValue = get_Mob_Rank_Value(csvTable.AsStringByIndex(col++));
	mobTable->bRank                = rankValue;

	int typeValue = get_Mob_Type_Value(csvTable.AsStringByIndex(col++));
	mobTable->bType                = typeValue;

	int battleTypeValue = get_Mob_BattleType_Value(csvTable.AsStringByIndex(col++));
	mobTable->bBattleType          = battleTypeValue;

	mobTable->bLevel		= atoi(csvTable.AsStringByIndex(col++));

	mobTable->bScalePct	= atoi(csvTable.AsStringByIndex(col++));

	int sizeValue = get_Mob_Size_Value(csvTable.AsStringByIndex(col++));
	mobTable->bSize                = sizeValue;

	int aiFlagValue = get_Mob_AIFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwAIFlag 	= aiFlagValue;

	mobTable->bMountCapacity = atoi(csvTable.AsStringByIndex(col++));

	int raceFlagValue = get_Mob_RaceFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwRaceFlag           = raceFlagValue;

	int immuneFlagValue = get_Mob_ImmuneFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwImmuneFlag         = immuneFlagValue;

	mobTable->bEmpire              = atoi(csvTable.AsStringByIndex(col++));

	strncpy(mobTable->szFolder, csvTable.AsStringByIndex(col++), sizeof(mobTable->szFolder));

	mobTable->bOnClickType         = atoi(csvTable.AsStringByIndex(col++));

	mobTable->bStr                 = atoi(csvTable.AsStringByIndex(col++));
	mobTable->bDex                 = atoi(csvTable.AsStringByIndex(col++));
	mobTable->bCon                 = atoi(csvTable.AsStringByIndex(col++));
	mobTable->bInt                 = atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwDamageRange[0]     = atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwDamageRange[1]     = atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwMaxHP              = atoi(csvTable.AsStringByIndex(col++));
	mobTable->bRegenCycle          = atoi(csvTable.AsStringByIndex(col++));
	mobTable->bRegenPercent        = atoi(csvTable.AsStringByIndex(col++));

	mobTable->dwGoldMin = atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwGoldMax = atoi(csvTable.AsStringByIndex(col++));

	mobTable->dwExp                = atoi(csvTable.AsStringByIndex(col++));
	mobTable->wDef                 = atoi(csvTable.AsStringByIndex(col++));
	mobTable->sAttackSpeed         = atoi(csvTable.AsStringByIndex(col++));
	mobTable->sMovingSpeed         = atoi(csvTable.AsStringByIndex(col++));
	mobTable->bAggresiveHPPct      = atoi(csvTable.AsStringByIndex(col++));
	mobTable->wAggressiveSight	= atoi(csvTable.AsStringByIndex(col++));
	mobTable->wAttackRange		= atoi(csvTable.AsStringByIndex(col++));

	mobTable->dwDropItemVnum	= atoi(csvTable.AsStringByIndex(col++));

	mobTable->dwResurrectionVnum = atoi(csvTable.AsStringByIndex(col++));

	for (int i = 0; i < CPythonNonPlayer::MOB_ENCHANTS_MAX_NUM; ++i)
		mobTable->cEnchants[i] = atoi(csvTable.AsStringByIndex(col++));

	for (int i = 0; i < CPythonNonPlayer::MOB_RESISTS_MAX_NUM; ++i)
		mobTable->cResists[i] = atoi(csvTable.AsStringByIndex(col++));

	for (int i = 0; i < CPythonNonPlayer::ATT_MAX_NUM; ++i)
		mobTable->cAttrs[i] = atoi(csvTable.AsStringByIndex(col++));

	mobTable->fDamMultiply		= atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwSummonVnum		= atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwDrainSP			= atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwMonsterColor	= atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwPolymorphItemVnum	= atoi(csvTable.AsStringByIndex(col++));

	for (int i = 0; i < CPythonNonPlayer::MOB_SKILL_MAX_NUM; ++i)
	{
		mobTable->Skills[i].bLevel = atoi(csvTable.AsStringByIndex(col++));
		mobTable->Skills[i].dwVnum = atoi(csvTable.AsStringByIndex(col++));
	}

	mobTable->bBerserkPoint	= atoi(csvTable.AsStringByIndex(col++));
	mobTable->bStoneSkinPoint	= atoi(csvTable.AsStringByIndex(col++));
	mobTable->bGodSpeedPoint	= atoi(csvTable.AsStringByIndex(col++));
	mobTable->bDeathBlowPoint	= atoi(csvTable.AsStringByIndex(col++));
	mobTable->bRevivePoint	= atoi(csvTable.AsStringByIndex(col++));

	mobTable->dwHealerPoint	= atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwHitRange	= atoi(csvTable.AsStringByIndex(col++));

	return true;
}

bool BuildMobTable()
{
	fprintf(stderr, "sizeof(TMobTable): %u\n", sizeof(CPythonNonPlayer::TMobTable));

	bool isNameFile = true;
	map<int,const char*> localMap;
	cCsvTable nameData;
	if(!nameData.Load("mob_names.txt",'\t'))
	{
		fprintf(stderr, "mob_names.txt Unable to read file\n");
		isNameFile = false;
	} else {
		nameData.Next();
		while(nameData.Next()) {
			localMap[atoi(nameData.AsStringByIndex(0))] = nameData.AsStringByIndex(1);
		}
	}

	set<int> vnumSet;

	map<DWORD, CPythonNonPlayer::TMobTable *> test_map_mobTableByVnum;

	//파일 읽어오기.
	cCsvTable data;
	if(!data.Load("mob_proto.txt",'\t'))
	{
		fprintf(stderr, "mob_proto.txt Unable to read file\n");
		return false;
	}
	data.Next(); //맨 윗줄 제외 (아이템 칼럼을 설명하는 부분)

	//===== 몹 테이블 생성=====//
	if (m_pMobTable)
	{
		delete m_pMobTable;
		m_pMobTable = NULL;
	}

	//새로 추가되는 갯수를 파악한다.
	int addNumber = 0;
	while(data.Next()) {
		int vnum = atoi(data.AsStringByIndex(0));
		std::map<DWORD, CPythonNonPlayer::TMobTable *>::iterator it_map_mobTable;
		it_map_mobTable = test_map_mobTableByVnum.find(vnum);
		if(it_map_mobTable != test_map_mobTableByVnum.end()) {
			addNumber++;
		}
	}


	m_iMobTableSize = data.m_File.GetRowCount()-1 + addNumber;

	m_pMobTable = new CPythonNonPlayer::TMobTable[m_iMobTableSize];
	memset(m_pMobTable, 0, sizeof(CPythonNonPlayer::TMobTable) * m_iMobTableSize);

	CPythonNonPlayer::TMobTable * mob_table = m_pMobTable;


	//data를 다시 첫줄로 옮긴다.(다시 읽어온다;;)
	data.Destroy();
	if(!data.Load("mob_proto.txt",'\t'))
	{
		fprintf(stderr, "mob_proto.txt Unable to read file\n");
		return false;
	}
	data.Next(); //맨 윗줄 제외 (아이템 칼럼을 설명하는 부분)

	while (data.Next())
	{
		int col = 0;
		//테스트 파일에 같은 vnum이 있는지 조사.
		std::map<DWORD, CPythonNonPlayer::TMobTable *>::iterator it_map_mobTable;
		it_map_mobTable = test_map_mobTableByVnum.find(atoi(data.AsStringByIndex(col)));
		if (it_map_mobTable == test_map_mobTableByVnum.end())
			if (!Set_Proto_Mob_Table(mob_table, data, localMap))
				fprintf(stderr, "Mob prototype table setting failed.\n");
	

		fprintf(stdout, "MOB #%-5d %-16s %-16s sight: %u color %u[%s]\n", mob_table->dwVnum, mob_table->szName, mob_table->szLocaleName, mob_table->wAggressiveSight, mob_table->dwMonsterColor, 0);

		//셋에 vnum 추가
		vnumSet.insert(mob_table->dwVnum);

		++mob_table;
	}

	return true;
}

DWORD g_adwMobProtoKey[4] =
{
	4813894,
	18955,
	552631,
	6822045
};

void SaveMobProto()
{
	FILE * fp;
	fopen_s(&fp, "mob_proto", "wb");
	if (!fp)
	{
		printf("cannot open %s for writing\n", "mob_proto");
		return;
	}

	DWORD fourcc = MAKEFOURCC('M', 'M', 'P', 'T');
	fwrite(&fourcc, sizeof(DWORD), 1, fp);

	DWORD dwElements = m_iMobTableSize;
	fwrite(&dwElements, sizeof(DWORD), 1, fp);

	CLZObject zObj;

	printf("sizeof(TMobTable) %d\n", sizeof(CPythonNonPlayer::TMobTable));

	if (!CLZO::instance().CompressEncryptedMemory(zObj, m_pMobTable, sizeof(CPythonNonPlayer::TMobTable) * m_iMobTableSize, g_adwMobProtoKey))
	{
		printf("cannot compress\n");
		fclose(fp);
		return;
	}

	const CLZObject::THeader & r = zObj.GetHeader();

	printf("MobProto count %u\n%u --Compress--> %u --Encrypt--> %u, GetSize %u\n",
			m_iMobTableSize, r.dwRealSize, r.dwCompressedSize, r.dwEncryptSize, zObj.GetSize());

	DWORD dwDataSize = zObj.GetSize();
	fwrite(&dwDataSize, sizeof(DWORD), 1, fp);
	fwrite(zObj.GetBuffer(), dwDataSize, 1, fp);

	fclose(fp);
}

void LoadMobProto()
{
	FILE * fp;
	DWORD fourcc, tableSize, dataSize;

	fopen_s(&fp, "mob_proto", "rb");
	if (fp==NULL)
	{
		printf("mob_proto not found\n");
		return;
	}

	fread(&fourcc, sizeof(DWORD), 1, fp);
	fread(&tableSize, sizeof(DWORD), 1, fp);
	fread(&dataSize, sizeof(DWORD), 1, fp);
	BYTE * data = (BYTE *) malloc(dataSize);

	printf("fourcc %u\n", fourcc);
	printf("tableSize %u\n", tableSize);
	printf("dataSize %u\n", dataSize);

	if (data)
	{
		fread(data, dataSize, 1, fp);

		CLZObject zObj;

		if (CLZO::instance().Decompress(zObj, data, g_adwMobProtoKey))
		{
			printf("real_size %u\n", zObj.GetSize());
			DWORD structSize = zObj.GetSize() / tableSize;
			DWORD structDiff = zObj.GetSize() % tableSize;
			printf("struct_size %u\n", structSize);
			printf("struct_diff %u\n", structDiff);

/*#ifdef ENABLE_PROTOSTRUCT_AUTODETECT
			if (structDiff!=0 && !CPythonNonPlayer::TMobTableAll::IsValidStruct(structSize))
#else
			if ((zObj.GetSize() % sizeof(TMobTable)) != 0)
#endif
			{
				printf("LoadMobProto: invalid size %u check data format. structSize %u, structDiff %u\n", zObj.GetSize(), structSize, structDiff);
				return;
			}*/


			if (LoadNPrint)
			{
				for (DWORD i = 0; i < tableSize; ++i)
				{
#ifdef ENABLE_PROTOSTRUCT_AUTODETECT
					CPythonNonPlayer::TMobTable rTable = {0};
					CPythonNonPlayer::TMobTableAll::Process(zObj.GetBuffer(), structSize, i, rTable);
#else
					CPythonNonPlayer::TMobTable & rTable = *((CItemData::TItemTable *) zObj.GetBuffer() + i);
#endif
					printf("%u %s\n", rTable.dwVnum, rTable.szLocaleName);
				}
			}
			else
			{
				FILE * mf1; fopen_s(&mf1, "mob_names.txt", "w");
				FILE * mf2; fopen_s(&mf2, "mob_proto.txt", "w");
				// FILE * mf3; fopen_s(&mf3, "mob_proto1.txt", "w");
				if (mf1==NULL)
				{
					printf("mob_names.txt not writable");
					return;
				}
				if (mf2==NULL)
				{
					printf("mob_proto.txt not writable");
					return;
				}
				fprintf(mf1, "VNUM\tLOCALE_NAME\n");
				fprintf(mf2, "VNUM\tNAME\tRANK\tTYPE\tBATTLE_TYPE\tLEVEL\tSCALE_PCT\tSIZE\tAI_FLAG\tMOUNT_CAPACITY\tRACE_FLAG\tIMMUNE_FLAG\tEMPIRE\tFOLDER\tON_CLICK\tST\tDX\tHT\tIQ\tDAMAGE_MIN\tDAMAGE_MAX\tMAX_HP\tREGEN_CYCLE\tREGEN_PERCENT\tGOLD_MIN\tGOLD_MAX\tEXP\tDEF\tATTACK_SPEED\tMOVE_SPEED\tAGGRESSIVE_HP_PCT\tAGGRESSIVE_SIGHT\tATTACK_RANGE\tDROP_ITEM\tRESURRECTION_VNUM\tENCHANT_CURSE\tENCHANT_SLOW\tENCHANT_POISON\tENCHANT_STUN\tENCHANT_CRITICAL\tENCHANT_PENETRATE\tRESIST_SWORD\tRESIST_TWOHAND\tRESIST_DAGGER\tRESIST_BELL\tRESIST_FAN\tRESIST_BOW\tRESIST_CLAW\tRESIST_FIRE\tRESIST_ELECT\tRESIST_MAGIC\tRESIST_WIND\tRESIST_ICE\tRESIST_EARTH\tRESIST_DARK\tRESIST_POISON\tRESIST_BLEEDING\tRESIST_FIST\tATT_ELEC\tATT_FIRE\tATT_ICE\tATT_WIND\tATT_EARTH\tATT_DARK\tDAM_MULTIPLY\tSUMMON\tDRAIN_SP\tMOB_COLOR\tPOLYMORPH_ITEM\tSKILL_LEVEL0\tSKILL_VNUM0\tSKILL_LEVEL1\tSKILL_VNUM1\tSKILL_LEVEL2\tSKILL_VNUM2\tSKILL_LEVEL3\tSKILL_VNUM3\tSKILL_LEVEL4\tSKILL_VNUM4\tSP_BERSERK\tSP_STONESKIN\tSP_GODSPEED\tSP_DEATHBLOW\tSP_REVIVE\tHEAL_VNUM\tHIT_RANGE\n");
				for (DWORD i = 0; i < tableSize; ++i)
				{
#ifdef ENABLE_PROTOSTRUCT_AUTODETECT
					CPythonNonPlayer::TMobTable rTable = {0};
					CPythonNonPlayer::TMobTableAll::Process(zObj.GetBuffer(), structSize, i, rTable);
#else
					CPythonNonPlayer::TMobTable & rTable = *((CItemData::TItemTable *) zObj.GetBuffer() + i);
#endif
					fprintf(mf1, "%u	%s\n", rTable.dwVnum, rTable.szLocaleName);
					// fprintf(mf3, "%d	%d	%d	%d	%d	%d	%d\n", rTable.cAttrs[CPythonNonPlayer::ATT_ELEC], rTable.cAttrs[CPythonNonPlayer::ATT_FIRE], rTable.cAttrs[CPythonNonPlayer::ATT_ICE], rTable.cAttrs[CPythonNonPlayer::ATT_WIND], rTable.cAttrs[CPythonNonPlayer::ATT_EARTH], rTable.cAttrs[CPythonNonPlayer::ATT_DARK], rTable.cResists[CPythonNonPlayer::MOB_RESIST_1]);
					fprintf(mf2,
						"%u	%s" //2
						"	%s	%s	%s	%u	%u" //7
						"	%s	%s	%u	%s" //11
						"	%s	%u	%s	%u	%u	%u	%u	%u" //19
						"	%u	%u	%u	%u	%u	%u	%u	%u	%u" //28
						"	%d	%d	%u	%u	%u	%u	%u" //35
						"	%d	%d	%d" //38
						"	%d	%d	%d" //41
						"	%d	%d	%d	%d" //45
						"	%d	%d	%d" //48
						"	%d	%d" //50
						"	%d	%d" //52
						"	%d	%d	%d" //55
						"	%d	%d	%d" //58
						"	%d	%d	%d" //61
						"	%d	%d	%d" //64
						"	%s	%u	%u	%u	%u" //67
						"	%u	%u	%u	%u	%u	%u" //75
						"	%u	%u	%u	%u" //79
						"	%u	%u	%u	%u	%u" //84
						"	%d	%u" //86
						"\n",
						rTable.dwVnum, rTable.szName, //2
						set_Mob_Rank_Value(rTable.bRank).c_str(), set_Mob_Type_Value(rTable.bType).c_str(), set_Mob_BattleType_Value(rTable.bBattleType).c_str(), rTable.bLevel, rTable.bScalePct, //7
						set_Mob_Size_Value(rTable.bSize).c_str(), set_Mob_AIFlag_Value(rTable.dwAIFlag).c_str(), rTable.bMountCapacity, set_Mob_RaceFlag_Value(rTable.dwRaceFlag).c_str(), //11
						set_Mob_ImmuneFlag_Value(rTable.dwImmuneFlag).c_str(), rTable.bEmpire, set_Mob_Char(rTable.szFolder).c_str(), rTable.bOnClickType, rTable.bStr, rTable.bDex, rTable.bCon, rTable.bInt, //19
						rTable.dwDamageRange[0], rTable.dwDamageRange[1], rTable.dwMaxHP, rTable.bRegenCycle, rTable.bRegenPercent, rTable.dwGoldMin, rTable.dwGoldMax, rTable.dwExp, rTable.wDef, //28
						rTable.sAttackSpeed, rTable.sMovingSpeed, rTable.bAggresiveHPPct, rTable.wAggressiveSight, rTable.wAttackRange, rTable.dwDropItemVnum, rTable.dwResurrectionVnum, //35
						rTable.cEnchants[CPythonNonPlayer::MOB_ENCHANT_CURSE], rTable.cEnchants[CPythonNonPlayer::MOB_ENCHANT_SLOW], rTable.cEnchants[CPythonNonPlayer::MOB_ENCHANT_POISON], //38
						rTable.cEnchants[CPythonNonPlayer::MOB_ENCHANT_STUN], rTable.cEnchants[CPythonNonPlayer::MOB_ENCHANT_CRITICAL], rTable.cEnchants[CPythonNonPlayer::MOB_ENCHANT_PENETRATE], //41
						rTable.cResists[CPythonNonPlayer::MOB_RESIST_SWORD], rTable.cResists[CPythonNonPlayer::MOB_RESIST_TWOHAND], rTable.cResists[CPythonNonPlayer::MOB_RESIST_DAGGER], rTable.cResists[CPythonNonPlayer::MOB_RESIST_BELL], //45
						rTable.cResists[CPythonNonPlayer::MOB_RESIST_FAN], rTable.cResists[CPythonNonPlayer::MOB_RESIST_BOW], rTable.cResists[CPythonNonPlayer::MOB_RESIST_CLAW], //48
						rTable.cResists[CPythonNonPlayer::MOB_RESIST_FIRE], rTable.cResists[CPythonNonPlayer::MOB_RESIST_ELECT], //50
						rTable.cResists[CPythonNonPlayer::MOB_RESIST_MAGIC], rTable.cResists[CPythonNonPlayer::MOB_RESIST_WIND], //52
						rTable.cResists[CPythonNonPlayer::MOB_RESIST_ICE], rTable.cResists[CPythonNonPlayer::MOB_RESIST_EARTH], rTable.cResists[CPythonNonPlayer::MOB_RESIST_DARK], //55
						rTable.cResists[CPythonNonPlayer::MOB_RESIST_POISON], rTable.cResists[CPythonNonPlayer::MOB_RESIST_BLEEDING], rTable.cResists[CPythonNonPlayer::MOB_RESIST_FIST], //58
						rTable.cAttrs[CPythonNonPlayer::ATT_ELEC], rTable.cAttrs[CPythonNonPlayer::ATT_FIRE], rTable.cAttrs[CPythonNonPlayer::ATT_ICE], //61
						rTable.cAttrs[CPythonNonPlayer::ATT_WIND], rTable.cAttrs[CPythonNonPlayer::ATT_EARTH], rTable.cAttrs[CPythonNonPlayer::ATT_DARK], //64
						set_Mob_Dam(rTable.fDamMultiply).c_str(), rTable.dwSummonVnum, rTable.dwDrainSP, rTable.dwMonsterColor, rTable.dwPolymorphItemVnum, //69
						rTable.Skills[0].bLevel, rTable.Skills[0].dwVnum, rTable.Skills[1].bLevel, rTable.Skills[1].dwVnum, rTable.Skills[2].bLevel, rTable.Skills[2].dwVnum, //75
						rTable.Skills[3].bLevel, rTable.Skills[3].dwVnum, rTable.Skills[4].bLevel, rTable.Skills[4].dwVnum, //79
						rTable.bBerserkPoint, rTable.bStoneSkinPoint, rTable.bGodSpeedPoint, rTable.bDeathBlowPoint, rTable.bRevivePoint, //84
						rTable.dwHealerPoint, rTable.dwHitRange //86
					);
				}
				;
			}
		}

		free(data);
	}

	fclose(fp);
}

string retrieveVnumRange(DWORD dwVnum, DWORD dwVnumRange)
{
	static char buf[10*2+1];
	if (dwVnumRange>0)
		snprintf(buf, sizeof(buf), "%u~%u", dwVnum, dwVnum+dwVnumRange);
	else
		snprintf(buf, sizeof(buf), "%u", dwVnum);
	return buf;
}

int retrieveAddonType(DWORD dwVnum)
{
	int addon_type = 0;
#ifdef ENABLE_ADDONTYPE_AUTODETECT
	static DWORD vnumlist[] = {180, 190, 290, 1130, 1170, 2150, 2170, 3160, 3210, 5110, 5120, 7160, 6010, 6060, 6070};
	for (DWORD i = 0; i < (sizeof(vnumlist)/sizeof(DWORD)); i++)
	{
		if ((dwVnum >= vnumlist[i]) && (dwVnum <= vnumlist[i]+9))
		{
			addon_type = -1;
		}
	}
#endif
	return addon_type;
}

bool Set_Proto_Item_Table(CItemData::TItemTable *itemTable, cCsvTable &csvTable, std::map<int,const char*> &nameMap)
{
	{
		std::string s(csvTable.AsStringByIndex(0));
		int pos = s.find("~");
		if (std::string::npos == pos)
		{
			itemTable->dwVnum = atoi(s.c_str());
			if (0 == itemTable->dwVnum)
			{
				printf ("INVALID VNUM %s\n", s.c_str());
				return false;
			}
			itemTable->dwVnumRange = 0;
		}
		else
		{
			std::string s_start_vnum (s.substr(0, pos));
			std::string s_end_vnum (s.substr(pos +1 ));

			int start_vnum = atoi(s_start_vnum.c_str());
			int end_vnum = atoi(s_end_vnum.c_str());
			if (0 == start_vnum || (0 != end_vnum && end_vnum < start_vnum))
			{
				printf ("INVALID VNUM RANGE%s\n", s.c_str());
				return false;
			}
			itemTable->dwVnum = start_vnum;
			itemTable->dwVnumRange = end_vnum - start_vnum;
		}
	}

	int col = 1;

	strncpy(itemTable->szName, csvTable.AsStringByIndex(col++), CItemData::ITEM_NAME_MAX_LEN);
	map<int,const char*>::iterator it;
	it = nameMap.find(itemTable->dwVnum);
	if (it != nameMap.end())
	{
		const char * localeName = it->second;
		strncpy(itemTable->szLocaleName, localeName, sizeof(itemTable->szLocaleName));
	}
	else
		strncpy(itemTable->szLocaleName, itemTable->szName, sizeof(itemTable->szLocaleName));

	itemTable->bType = get_Item_Type_Value(csvTable.AsStringByIndex(col++));
	itemTable->bSubType = get_Item_SubType_Value(itemTable->bType, csvTable.AsStringByIndex(col++));
	itemTable->bMaskType = get_Item_MaskType_Value(csvTable.AsStringByIndex(col++));
	itemTable->bMaskSubType = get_Item_MaskSubType_Value(itemTable->bMaskType, csvTable.AsStringByIndex(col++));
	itemTable->bSize = atoi(csvTable.AsStringByIndex(col++));
	itemTable->dwAntiFlags = get_Item_AntiFlag_Value(csvTable.AsStringByIndex(col++));
	itemTable->dwFlags = get_Item_Flag_Value(csvTable.AsStringByIndex(col++));
	itemTable->dwWearFlags = get_Item_WearFlag_Value(csvTable.AsStringByIndex(col++));
	itemTable->dwImmuneFlag = get_Item_Immune_Value(csvTable.AsStringByIndex(col++));
	itemTable->dwIBuyItemPrice = atoi(csvTable.AsStringByIndex(col++));
	itemTable->dwISellItemPrice = atoi(csvTable.AsStringByIndex(col++));
	itemTable->dwRefinedVnum = atoi(csvTable.AsStringByIndex(col++));
	itemTable->wRefineSet = atoi(csvTable.AsStringByIndex(col++));
	itemTable->bAlterToMagicItemPct = atoi(csvTable.AsStringByIndex(col++));

	int i;

	for (i = 0; i < CItemData::ITEM_LIMIT_MAX_NUM; ++i)
	{
		itemTable->aLimits[i].bType = get_Item_LimitType_Value(csvTable.AsStringByIndex(col++));
		itemTable->aLimits[i].lValue = atoi(csvTable.AsStringByIndex(col++));
	}

	for (i = 0; i < CItemData::ITEM_APPLY_MAX_NUM; ++i)
	{
		itemTable->aApplies[i].bType = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col++));
		itemTable->aApplies[i].lValue = atoi(csvTable.AsStringByIndex(col++));
	}

	for (i = 0; i < CItemData::ITEM_VALUES_MAX_NUM; ++i)
		itemTable->alValues[i] = atoi(csvTable.AsStringByIndex(col++));

	itemTable->bSpecular = atoi(csvTable.AsStringByIndex(col++));
	itemTable->bGainSocketPct = atoi(csvTable.AsStringByIndex(col++));

	col++; //AddonType

	itemTable->bWeight = 0;

	return true;
}

bool BuildItemTable()
{
	fprintf(stderr, "sizeof(TClientItemTable): %u\n", sizeof(CItemData::TItemTable));

	bool isNameFile = true;
	map<int,const char*> localMap;
	cCsvTable nameData;
	if(!nameData.Load("item_names.txt",'\t'))
	{
		fprintf(stderr, "item_names.txt Unable to read file\n");
		isNameFile = false;
	}
	else
	{
		nameData.Next();
		while(nameData.Next())
		{
			localMap[atoi(nameData.AsStringByIndex(0))] = nameData.AsStringByIndex(1);
		}
	}

	map<DWORD, CItemData::TItemTable *> test_map_itemTableByVnum;
	set<int> vnumSet;

	cCsvTable data;

	if (!data.Load("item_proto.txt",'\t'))
	{
		fprintf(stderr, "item_proto.txt Unable to read file\n");
		return false;
	}
	data.Next();

	if (m_pItemTable)
	{
		free(m_pItemTable);
		m_pItemTable = NULL;
	}

	int addNumber = 0;
	while(data.Next())
	{
		int vnum = atoi(data.AsStringByIndex(0));
		std::map<DWORD, CItemData::TItemTable *>::iterator it_map_itemTable;
		it_map_itemTable = test_map_itemTableByVnum.find(vnum);
		if (it_map_itemTable != test_map_itemTableByVnum.end())
			addNumber++;
	}

	data.Destroy();
	if(!data.Load("item_proto.txt",'\t'))
	{
		fprintf(stderr, "item_proto.txt Unable to read file\n");
		return false;
	}
	data.Next();

	m_iItemTableSize = data.m_File.GetRowCount()-1+addNumber;
	m_pItemTable = new CItemData::TItemTable[m_iItemTableSize];
	memset(m_pItemTable, 0, sizeof(CItemData::TItemTable) * m_iItemTableSize);

	CItemData::TItemTable * item_table = m_pItemTable;

	while (data.Next())
	{
		int col = 0;

		std::map<DWORD, CItemData::TItemTable *>::iterator it_map_itemTable;
		it_map_itemTable = test_map_itemTableByVnum.find(atoi(data.AsStringByIndex(col)));
		if (!Set_Proto_Item_Table(item_table, data, localMap))
			fprintf(stderr, "Mob prototype table setting failed.\n");

		fprintf(stdout, "ITEM #%-5u %-24s %-24s VAL: %ld %ld %ld %ld %ld %ld WEAR %u ANTI %u IMMUNE %u REFINE %u\n",
				item_table->dwVnum,
				item_table->szName,
				item_table->szLocaleName,
				item_table->alValues[0],
				item_table->alValues[1],
				item_table->alValues[2],
				item_table->alValues[3],
				item_table->alValues[4],
				item_table->alValues[5],
				item_table->dwWearFlags,
				item_table->dwAntiFlags,
				item_table->dwImmuneFlag,
				item_table->dwRefinedVnum);

		vnumSet.insert(item_table->dwVnum);
		++item_table;
	}

	return true;
}

DWORD g_adwItemProtoKey[4] =
{
	// 173217,
	// 72619434,
	// 408587239,
	// 27973291
	1411666002,
	2304517176,
	2304517192,
	
};

void LoadItemProto()
{
	FILE * fp;
	DWORD fourcc, tableSize, dataSize, protoVersion, structSize;

	fopen_s(&fp, "item_proto", "rb");
	if (fp==NULL)
	{
		printf("item_proto not found\n");
		return;
	}
	

	fread(&fourcc, sizeof(DWORD), 1, fp);
	fread(&protoVersion, sizeof(DWORD), 1, fp);
	fread(&structSize, sizeof(DWORD), 1, fp);
	fread(&tableSize, sizeof(DWORD), 1, fp);
	fread(&dataSize, sizeof(DWORD), 1, fp);
	BYTE * data = (BYTE *) malloc(dataSize);

	printf("fourcc %u\n", fourcc);
	printf("protoVersion %u\n", protoVersion);
	printf("struct_size %u\n", structSize);
	printf("tableSize %u\n", tableSize);
	printf("dataSize %u\n", dataSize);

// #ifdef ENABLE_PROTOSTRUCT_AUTODETECT
	// if (!CItemData::TItemTableAll::IsValidStruct(structSize))
// #else
	// if (structSize != sizeof(CItemData::TItemTable))
// #endif
	// {
		// printf("LoadItemProto: invalid item_proto structSize[%d] != sizeof(SItemTable)\n", structSize, sizeof(CItemData::TItemTable));
		// return;
	// }

	if (data)
	{
		fread(data, dataSize, 1, fp);

		CLZObject zObj;

		if (CLZO::instance().Decompress(zObj, data, g_adwItemProtoKey))
		{
			printf("real_size %u\n", zObj.GetSize());

			if (LoadNPrint)
			{
				for (DWORD i = 0; i < tableSize; ++i)
				{
#ifdef ENABLE_PROTOSTRUCT_AUTODETECT
					CItemData::TItemTable rTable = {0};
					CItemData::TItemTableAll::Process(zObj.GetBuffer(), structSize, i, rTable);
#else
					CItemData::TItemTable & rTable = *((CItemData::TItemTable *) zObj.GetBuffer() + i);
#endif
					printf("%u %s\n", rTable.dwVnum, rTable.szLocaleName);
				}
			}
			else
			{
				FILE * mf1; fopen_s(&mf1, "item_names.txt", "w");
				FILE * mf2; fopen_s(&mf2, "item_proto.txt", "w");
				// FILE * mf3; fopen_s(&mf3, "i1tem_proto.txt", "w");
				if (mf1==NULL)
				{
					printf("item_names.txt not writable");
					return;
				}
				if (mf2==NULL)
				{
					printf("item_proto.txt not writable");
					return;
				}
				fprintf(mf1, "VNUM\tLOCALE_NAME\n");
				fprintf(mf2, "ITEM_VNUM~RANGE\tITEM_NAME(K)\tITEM_TYPE\tSUB_TYPE\tMASKED_TYPE\tMASKED_SUB_TYPE\tSIZE\tANTI_FLAG\tFLAG\tITEM_WEAR\tIMMUNE\tGOLD\tSHOP_BUY_PRICE\tREFINE\tREFINESET\tMAGIC_PCT\tLIMIT_TYPE0\tLIMIT_VALUE0\tLIMIT_TYPE1\tLIMIT_VALUE1\tADDON_TYPE0\tADDON_VALUE0\tADDON_TYPE1\tADDON_VALUE1\tADDON_TYPE2\tADDON_VALUE2\tADDON_TYPE3\tADDON_VALUE3\tVALUE0\tVALUE1\tVALUE2\tVALUE3\tVALUE4\tVALUE5\tSpecular\tSOCKET\tATTU_ADDON\n");
				for (DWORD i = 0; i < tableSize; ++i)
				{
#ifdef ENABLE_PROTOSTRUCT_AUTODETECT
					CItemData::TItemTable rTable = {0};
					CItemData::TItemTableAll::Process(zObj.GetBuffer(), structSize, i, rTable);
#else
					CItemData::TItemTable & rTable = *((CItemData::TItemTable *) zObj.GetBuffer() + i);
#endif
					fprintf(mf1, "%u	%s\n", rTable.dwVnum, rTable.szLocaleName);
					// fprintf(mf3, "%d	%d\n", rTable.bMaskType, rTable.bMaskSubType);
					fprintf(mf2,
						"%s	%s" // 2
						"	%s	%s	%s	%s	%u	%s" // 8
						"	%s	%s	%s" // 11
						"	%u	%u	%u	%u	%u" // 16
						"	%s	%d	%s	%d" // 20
						"	%s	%d	%s	%d	%s	%d	%s	%d" // 28
						"	%d	%d	%d	%d	%d	%d" // 34
						"	%u	%u	%d" // 37
						"\n",
						retrieveVnumRange(rTable.dwVnum, rTable.dwVnumRange).c_str(), rTable.szName, // 2
						set_Item_Type_Value(rTable.bType).c_str(), set_Item_SubType_Value(rTable.bType, rTable.bSubType).c_str(), set_Item_MaskType_Value(rTable.bMaskType).c_str(), set_Item_MaskSubType_Value(rTable.bMaskType, rTable.bMaskSubType).c_str(), rTable.bSize, set_Item_AntiFlag_Value(rTable.dwAntiFlags).c_str(), // 8
						set_Item_Flag_Value(rTable.dwFlags).c_str(), set_Item_WearFlag_Value(rTable.dwWearFlags).c_str(), set_Item_Immune_Value(rTable.dwImmuneFlag).c_str(), // 11
						rTable.dwIBuyItemPrice, rTable.dwISellItemPrice, rTable.dwRefinedVnum, rTable.wRefineSet, rTable.bAlterToMagicItemPct, // 16
						set_Item_LimitType_Value(rTable.aLimits[0].bType).c_str(), rTable.aLimits[0].lValue, set_Item_LimitType_Value(rTable.aLimits[1].bType).c_str(), rTable.aLimits[1].lValue, // 20
						set_Item_ApplyType_Value(rTable.aApplies[0].bType).c_str(), rTable.aApplies[0].lValue, set_Item_ApplyType_Value(rTable.aApplies[1].bType).c_str(), rTable.aApplies[1].lValue, // 24
						set_Item_ApplyType_Value(rTable.aApplies[2].bType).c_str(), rTable.aApplies[2].lValue, set_Item_ApplyType_Value(rTable.aApplies[3].bType).c_str(), rTable.aApplies[3].lValue, // 28
						rTable.alValues[0], rTable.alValues[1], rTable.alValues[2], rTable.alValues[3], rTable.alValues[4], rTable.alValues[5], // 34
						rTable.bSpecular, rTable.bGainSocketPct, retrieveAddonType(rTable.dwVnum) // 37
					);
				}
				;
			}
		}

		free(data);
	}

	fclose(fp);
}

void SaveItemProto()
{
	FILE * fp;
	fopen_s(&fp, "item_proto", "wb");
	if (!fp)
	{
		printf("cannot open %s for writing\n", "item_proto");
		return;
	}

	DWORD fourcc = MAKEFOURCC('M', 'I', 'P', 'X');
	fwrite(&fourcc, sizeof(DWORD), 1, fp);

	DWORD dwVersion = 0x00000001;
	fwrite(&dwVersion, sizeof(DWORD), 1, fp);

	DWORD dwStride = sizeof(CItemData::TItemTable);
	fwrite(&dwStride, sizeof(DWORD), 1, fp);

	DWORD dwElements = m_iItemTableSize;
	fwrite(&dwElements, sizeof(DWORD), 1, fp);

	CLZObject zObj;
	std::vector <CItemData::TItemTable> vec_item_table (&m_pItemTable[0], &m_pItemTable[m_iItemTableSize - 1]);
	sort (&m_pItemTable[0], &m_pItemTable[0] + m_iItemTableSize);
	if (!CLZO::instance().CompressEncryptedMemory(zObj, m_pItemTable, sizeof(CItemData::TItemTable) * m_iItemTableSize, g_adwItemProtoKey))
	{
		printf("cannot compress\n");
		fclose(fp);
		return;
	}

	const CLZObject::THeader & r = zObj.GetHeader();

	printf("Elements %d\n%u --Compress--> %u --Encrypt--> %u, GetSize %u\n",
			m_iItemTableSize,
			r.dwRealSize,
			r.dwCompressedSize,
			r.dwEncryptSize,
			zObj.GetSize());

	DWORD dwDataSize = zObj.GetSize();
	fwrite(&dwDataSize, sizeof(DWORD), 1, fp);
	fwrite(zObj.GetBuffer(), dwDataSize, 1, fp);
	fclose(fp);

	fopen_s(&fp, "item_proto", "rb");
	if (!fp)
	{
		printf("Error!!\n");
		return;
	}

	fread(&fourcc, sizeof(DWORD), 1, fp);
	fread(&dwElements, sizeof(DWORD), 1, fp);

	printf("Elements Check %u fourcc match %d\n", dwElements, fourcc == MAKEFOURCC('M', 'I', 'P', 'T'));
	fclose(fp);
}

int main(void)
{
	setlocale(LC_ALL, "Turkish");
	printf("Lütfen Yapmak İstediğiniz İşlemi Seçiniz:\n");
	printf("Açma : 0\n");
	printf("Kapatma : 1\n");
	printf("Struct Size : 2\n");
	BYTE a, b;
	bas:
	scanf("%d", &a);

#ifdef _DEBUG
	printf("sizeof(TItemTable) %d\n", sizeof(CItemData::TItemTable));
	printf("sizeof(TMobTable) %d\n", sizeof(CPythonNonPlayer::TMobTable));
#endif

	switch(a)
	{
		case 0:
			printf("Lütfen Açmak İstediğiniz Protoyu Seçiniz:\n");
			printf("İtem : 0\n");
			printf("Mob : 1\n");
			printf("Her İkisi : 2\n");
			bas1:
			scanf("%d", &b);
			switch(b)
			{
				case 0:
					LoadItemProto();
					break;	
				case 1:
					LoadMobProto();
					break;
				case 2:
					LoadItemProto();
					LoadMobProto();
					break;
				default:
					printf("Lütfen Geçerli İşlemi Seçiniz !!");
					goto bas1;
			}
			break;
		case 1:
			printf("Lütfen Kapatmak İstediğiniz Protoyu Seçiniz:\n");
			printf("İtem : 0\n");
			printf("Mob : 1\n");
			printf("Her İkisi : 2\n");
			bas2:
			scanf("%d", &b);
			switch(b)
			{
				case 0:
					if (BuildItemTable())
						SaveItemProto();
					break;
				case 1:
					if (BuildMobTable())
						SaveMobProto();
					break;
				case 2:
					if (BuildItemTable())
						SaveItemProto();
					if (BuildMobTable())
						SaveMobProto();
					break;
				default:
					printf("Lütfen Geçerli İşlemi Seçiniz !!");
					goto bas2;
			}
			break;
		case 2:
			printf("İtem: %d\n", sizeof(CItemData::TItemTable));
			printf("Mob: %d\n", sizeof(CPythonNonPlayer::TMobTable));
			break;
		default:
			printf("Lütfen Geçerli İşlemi Seçiniz !!");
			goto bas;
	}

	system("Pause");
	return 0;
}

