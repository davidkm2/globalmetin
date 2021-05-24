#include <map>
#include "stdafx.h"
#include "ClientManager.h"
#include "Main.h"
#include "CsvReader.h"
#include "ProtoReader.h"

using namespace std;

bool CClientManager::InitializeTables()
{
	if (!InitializeMobTable())
	{
		sys_err("InitializeMobTable FAILED");
		return false;
	}

	if (!MirrorMobTableIntoDB())
	{
		sys_err("MirrorMobTableIntoDB FAILED");
		return false; 
	}

	if (!InitializeItemTable())
	{
		sys_err("InitializeItemTable FAILED");
		return false; 
	}

	if (!MirrorItemTableIntoDB())
	{
		sys_err("MirrorItemTableIntoDB FAILED");
		return false; 
	}

	if (!InitializeShopTable())
	{
		sys_err("InitializeShopTable FAILED");
		return false;
	}

#ifdef __GEM__
	if (!InitializeGemShopTable())
	{
		sys_err("InitializeGemShopTable FAILED");
		return false;
	}
#endif

	if (!InitializeSkillTable())
	{
		sys_err("InitializeSkillTable FAILED");
		return false;
	}

	if (!InitializeRefineTable())
	{
		sys_err("InitializeRefineTable FAILED");
		return false;
	}

	if (!InitializeItemAttrTable())
	{
		sys_err("InitializeItemAttrTable FAILED");
		return false;
	}

	if (!InitializeItemRareTable())
	{
		sys_err("InitializeItemRareTable FAILED");
		return false;
	}

	if (!InitializeBanwordTable())
	{
		sys_err("InitializeBanwordTable FAILED");
		return false;
	}

	if (!InitializeLandTable())
	{
		sys_err("InitializeLandTable FAILED");
		return false;
	}

	if (!InitializeObjectProto())
	{
		sys_err("InitializeObjectProto FAILED");
		return false;
	}

	if (!InitializeObjectTable())
	{
		sys_err("InitializeObjectTable FAILED");
		return false;
	}

	return true;
}

bool CClientManager::InitializeRefineTable()
{
	char query[2048];

	snprintf(query, sizeof(query),
			"SELECT id, cost, prob, vnum0, count0, vnum1, count1, vnum2, count2,  vnum3, count3, vnum4, count4 FROM refine_proto%s",
			GetTablePostfix());

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult * pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
		return true;

	if (m_pRefineTable)
	{
		sys_log(0, "RELOAD: refine_proto");
		delete [] m_pRefineTable;
		m_pRefineTable = NULL;
	}

	m_iRefineTableSize = pRes->uiNumRows;

	m_pRefineTable	= new TRefineTable[m_iRefineTableSize];
	memset(m_pRefineTable, 0, sizeof(TRefineTable) * m_iRefineTableSize);

	TRefineTable* prt = m_pRefineTable;
	MYSQL_ROW data;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		int col = 0;
		str_to_number(prt->id, data[col++]);
		str_to_number(prt->cost, data[col++]);
		str_to_number(prt->prob, data[col++]);

		for (int i = 0; i < REFINE_MATERIAL_MAX_NUM; i++)
		{
			str_to_number(prt->materials[i].vnum, data[col++]);
			str_to_number(prt->materials[i].count, data[col++]);
			if (!prt->materials[i].vnum)
			{
				prt->material_count = i;
				break;
			}
		}

		sys_log(0, "REFINE: id %ld cost %d prob %d mat1 %lu cnt1 %d", prt->id, prt->cost, prt->prob, prt->materials[0].vnum, prt->materials[0].count);

		prt++;
	}
	return true;
}

class FCompareVnum
{
	public:
		bool operator () (const TEntityTable & a, const TEntityTable & b) const
		{
			return (a.dwVnum < b.dwVnum);
		}
};

bool CClientManager::InitializeMobTable()
{
	map<int,const char*> localMap;
	cCsvTable nameData;
	if (!nameData.Load("mob_names.txt",'\t'))
		fprintf(stderr, "Could not load mob_names.txt\n");
	else
	{
		nameData.Next();
		while(nameData.Next())
			localMap[atoi(nameData.AsStringByIndex(0))] = nameData.AsStringByIndex(1);
	}
#ifdef __EDIT_PROTO__
	cCsvTable nameData2;
	if (!nameData2.Load("mob_names2.txt",'\t'))
		fprintf(stderr, "Could not load mob_names2.txt\n");
	else
	{
		nameData2.Next();
		while(nameData2.Next())
			localMap[atoi(nameData2.AsStringByIndex(0))] = nameData2.AsStringByIndex(1);
	}

	cCsvTable data2;

	if(!data2.Load("mob_proto2.txt",'\t'))
	{
		fprintf(stderr, "Could not load mob_proto2.txt. Wrong file format?\n");
		return false;
	}
	data2.Next();

	cCsvTable data3;
	if(!data3.Load("mob_proto_edit.txt",'\t'))
	{
		fprintf(stderr, "Could not load mob_proto_edit.txt. Wrong file format?\n");
		return false;
	}
	data3.Next();
#endif

	cCsvTable data;

	if(!data.Load("mob_proto.txt",'\t'))
	{
		fprintf(stderr, "Could not load mob_proto.txt. Wrong file format?\n");
		return false;
	}
	data.Next();

	if (!m_vec_mobTable.empty())
	{
		sys_log(0, "RELOAD: mob_proto");
		m_vec_mobTable.clear();
	}
	m_vec_mobTable.resize(data.m_File.GetRowCount()-1
#ifdef __EDIT_PROTO__
+ data2.m_File.GetRowCount()-1
#endif
);
	memset(&m_vec_mobTable[0], 0, sizeof(TMobTable) * m_vec_mobTable.size());
	TMobTable * mob_table = &m_vec_mobTable[0];

	while (data.Next())
	{
#ifdef __EDIT_PROTO__
		data3.Next();
#endif
		if (!Set_Proto_Mob_Table(mob_table, data,
#ifdef __EDIT_PROTO__
data3,
#endif
localMap))
			fprintf(stderr, "Could not process entry.\n");

		sys_log(1, "MOB #%-5d %-24s %-24s level: %-3u rank: %u empire: %d", mob_table->dwVnum, mob_table->szName, mob_table->szLocaleName, mob_table->bLevel, mob_table->bRank, mob_table->bEmpire);
		++mob_table;

	}
#ifdef __EDIT_PROTO__
	while (data2.Next())
	{
		data3.Next();
		if (!Set_Proto_Mob_Table(mob_table, data2, data3, localMap))
			fprintf(stderr, "Could not process entry.\n");

		sys_log(1, "MOB #%-5d %-24s %-24s level: %-3u rank: %u empire: %d", mob_table->dwVnum, mob_table->szName, mob_table->szLocaleName, mob_table->bLevel, mob_table->bRank, mob_table->bEmpire);
		++mob_table;

	}
#endif
	sort(m_vec_mobTable.begin(), m_vec_mobTable.end(), FCompareVnum());
	return true;
}

#ifdef __GEM__
bool CClientManager::InitializeGemShopTable()
{
	MYSQL_ROW	data;
	int		col;

	static const char * s_szQuery = "SELECT id, vnum, count, price, row FROM gem_shop ORDER BY id";
	std::unique_ptr<SQLMsg> pkMsg2(CDBManager::instance().DirectQuery(s_szQuery));

	SQLResult * pRes2 = pkMsg2->Get();

	if (!pRes2->uiNumRows)
	{
		sys_err("InitializeGemShopTable : Table count is zero.");
		return false;
	}

	std::map<int, TGemShopTable *> map_shop;

	if (m_pGemShopTable)
	{
		delete [] (m_pGemShopTable);
		m_pGemShopTable = NULL;
	}

	TGemShopTable * shop_table = m_pGemShopTable;

	while ((data = mysql_fetch_row(pRes2->pSQLResult)))
	{
		col = 0;
		
		int iShopId = 0;
		str_to_number(iShopId, data[col++]);

		if (map_shop.end() == map_shop.find(iShopId))
		{
			shop_table = new TGemShopTable;
			memset(shop_table, 0, sizeof(TGemShopTable));

			str_to_number(shop_table->dwVnum, data[col++]);
			str_to_number(shop_table->bCount, data[col++]);
			str_to_number(shop_table->dwPrice, data[col++]);
			str_to_number(shop_table->dwRow, data[col++]);
			
			map_shop[iShopId] = shop_table;
		}
		else
		{
			shop_table = map_shop[iShopId];
		}

		if (!data[col])
			continue;
	}

	m_pGemShopTable = new TGemShopTable[map_shop.size()];
	m_iGemShopTableSize = map_shop.size();

	int i = 0;

	auto it = map_shop.begin();
	while (it != map_shop.end())
	{
		memcpy((m_pGemShopTable + i), (it++)->second, sizeof(TGemShopTable));
		++i;
	}

	return true;
}
#endif

bool CClientManager::InitializeShopTable()
{
	MYSQL_ROW	data;
	int		col;

	static const char * s_szQuery =
		"SELECT "
		"shop.vnum, "
		"shop.npc_vnum, "
		"shop_item.item_vnum, "
		"shop_item.count, "
		"shop_item.gold, "
#ifdef __CHEQUE__
		"shop_item.cheque, "
#endif
#ifdef __BATTLE_FIELD__
		"shop_item.bfpoint "
#endif
		"FROM shop LEFT JOIN shop_item "
		"ON shop.vnum = shop_item.shop_vnum ORDER BY shop.vnum, shop_item.item_vnum";

	std::unique_ptr<SQLMsg> pkMsg2(CDBManager::instance().DirectQuery(s_szQuery));

	SQLResult * pRes2 = pkMsg2->Get();

	if (!pRes2->uiNumRows)
	{
		sys_err("InitializeShopTable : Table count is zero.");
		return false;
	}

	std::map<int, TShopTable *> map_shop;

	if (m_pShopTable)
	{
		delete [] (m_pShopTable);
		m_pShopTable = NULL;
	}

	TShopTable * shop_table = m_pShopTable;

	while ((data = mysql_fetch_row(pRes2->pSQLResult)))
	{
		col = 0;

		int iShopVnum = 0;
		str_to_number(iShopVnum, data[col++]);

		if (map_shop.end() == map_shop.find(iShopVnum))
		{
			shop_table = new TShopTable;
			memset(shop_table, 0, sizeof(TShopTable));
			shop_table->dwVnum	= iShopVnum;

			map_shop[iShopVnum] = shop_table;
		}
		else
			shop_table = map_shop[iShopVnum];

		str_to_number(shop_table->dwNPCVnum, data[col++]);

		if (!data[col])
			continue;

		TShopItemTable * pItem = &shop_table->items[shop_table->byItemCount];

		str_to_number(pItem->vnum, data[col++]);
		str_to_number(pItem->count, data[col++]);
		for (int i = 0; i < SHOP_PRICE_MAX_NUM; ++i)
			str_to_number(pItem->dwPrices[i], data[col++]);
		++shop_table->byItemCount;
	}

	m_pShopTable = new TShopTable[map_shop.size()];
	m_iShopTableSize = map_shop.size();

	int i = 0;

	auto it = map_shop.begin();
	while (it != map_shop.end())
	{
		memcpy((m_pShopTable + i), (it++)->second, sizeof(TShopTable));
		sys_log(0, "SHOP: #%d items: %d", (m_pShopTable + i)->dwVnum, (m_pShopTable + i)->byItemCount);
		++i;
	}

	return true;
}

bool CClientManager::InitializeQuestItemTable()
{
	using namespace std;

	static const char * s_szQuery = "SELECT vnum, name, %s FROM quest_item_proto ORDER BY vnum";

	char query[1024];
	snprintf(query, sizeof(query), s_szQuery, "locale_name");

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult * pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
	{
		sys_err("query error or no rows: %s", query);
		return false;
	}

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pRes->pSQLResult)))
	{
		int col = 0;

		TItemTable tbl;
		memset(&tbl, 0, sizeof(tbl));

		str_to_number(tbl.dwVnum, row[col++]);

		if (row[col])
			strlcpy(tbl.szName, row[col], sizeof(tbl.szName));

		col++;

		if (row[col])
			strlcpy(tbl.szLocaleName, row[col], sizeof(tbl.szLocaleName));

		col++;

		if (m_map_itemTableByVnum.find(tbl.dwVnum) != m_map_itemTableByVnum.end())
		{
			sys_err("QUEST_ITEM_ERROR! %lu vnum already exist! (name %s)", tbl.dwVnum, tbl.szLocaleName);
			continue;
		}

		tbl.bType = ITEM_QUEST;
		tbl.bSize = 1;

		m_vec_itemTable.push_back(tbl);
	}

	return true;
}

bool CClientManager::InitializeItemTable()
{
	map<int,const char*> localMap;
	cCsvTable nameData;
	if(!nameData.Load("item_names.txt",'\t'))
		fprintf(stderr, "Could not load item_names.txt.\n");
	else
	{
		nameData.Next();
		while(nameData.Next())
			localMap[atoi(nameData.AsStringByIndex(0))] = nameData.AsStringByIndex(1);
	}
#ifdef __EDIT_PROTO__
	cCsvTable nameData2;
	if (!nameData2.Load("item_names2.txt",'\t'))
		fprintf(stderr, "Could not load item_names2.txt.\n");
	else
	{
		nameData2.Next();
		while(nameData2.Next())
			localMap[atoi(nameData2.AsStringByIndex(0))] = nameData2.AsStringByIndex(1);
	}

	cCsvTable data2;
	if(!data2.Load("item_proto2.txt",'\t'))
	{
		fprintf(stderr, "Could not load item_proto2.txt. Wrong file format?\n");
		return false;
	}
	data2.Next();
#endif
	cCsvTable data;
	if(!data.Load("item_proto.txt",'\t'))
	{
		fprintf(stderr, "Could not load item_proto.txt. Wrong file format?\n");
		return false;
	}
	data.Next();

	if (!m_vec_itemTable.empty())
	{
		sys_log(0, "RELOAD: item_proto");
		m_vec_itemTable.clear();
		m_map_itemTableByVnum.clear();
	}

	data.Destroy();
	if(!data.Load("item_proto.txt",'\t'))
	{
		fprintf(stderr, "Could not load item_proto.txt. Wrong file format?\n");
		return false;
	}
	data.Next();

	m_vec_itemTable.resize(data.m_File.GetRowCount() - 1
#ifdef __EDIT_PROTO__
+ data2.m_File.GetRowCount() - 1
#endif
);
	memset(&m_vec_itemTable[0], 0, sizeof(TItemTable) * m_vec_itemTable.size());

	TItemTable * item_table = &m_vec_itemTable[0];

	while (data.Next())
	{
		if (!Set_Proto_Item_Table(item_table, data, localMap))
			fprintf(stderr, "Failed to load item_proto table.\n");

		m_map_itemTableByVnum.insert(std::map<DWORD, TItemTable *>::value_type(item_table->dwVnum, item_table));
		++item_table;
	}
#ifdef __EDIT_PROTO__
	while (data2.Next())
	{
		if (!Set_Proto_Item_Table(item_table, data2, localMap))
			fprintf(stderr, "Failed to load item_proto2 table.\n");

		m_map_itemTableByVnum.insert(std::map<DWORD, TItemTable *>::value_type(item_table->dwVnum, item_table));
		++item_table;
	}
#endif
	m_map_itemTableByVnum.clear();

	itertype(m_vec_itemTable) it = m_vec_itemTable.begin();

	while (it != m_vec_itemTable.end())
	{
		TItemTable * item_table = &(*(it++));

		sys_log(1, "ITEM: #%-5lu %-24s %-24s VAL: %ld %ld %ld %ld %ld %ld WEAR %lu ANTI %lu IMMUNE %lu REFINE %lu REFINE_SET %u MAGIC_PCT %u",
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
				item_table->dwRefinedVnum,
				item_table->wRefineSet,
				item_table->bAlterToMagicItemPct);

		m_map_itemTableByVnum.insert(std::map<DWORD, TItemTable *>::value_type(item_table->dwVnum, item_table));
	}
	sort(m_vec_itemTable.begin(), m_vec_itemTable.end(), FCompareVnum());
	return true;
}

bool CClientManager::InitializeSkillTable()
{
	char query[4096];
	snprintf(query, sizeof(query),
		"SELECT dwVnum, szName, bType, bMaxLevel, dwSplashRange, "
		"szPointOn, szPointPoly, szSPCostPoly, szDurationPoly, szDurationSPCostPoly, "
		"szCooldownPoly, szMasterBonusPoly, setFlag+0, setAffectFlag+0, "
		"szPointOn2, szPointPoly2, szDurationPoly2, setAffectFlag2+0, "
		"szPointOn3, szPointPoly3, szDurationPoly3, szGrandMasterAddSPCostPoly, "
		"bLevelStep, bLevelLimit, prerequisiteSkillVnum, prerequisiteSkillLevel, iMaxHit, szSplashAroundDamageAdjustPoly, eSkillType+0, dwTargetRange "
		"FROM skill_proto%s ORDER BY dwVnum",
		GetTablePostfix());

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult * pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
	{
		sys_err("no result from skill_proto");
		return false;
	}

	if (!m_vec_skillTable.empty())
	{
		sys_log(0, "RELOAD: skill_proto");
		m_vec_skillTable.clear();
	}

	m_vec_skillTable.reserve(pRes->uiNumRows);

	MYSQL_ROW	data;
	int		col;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		TSkillTable t;
		memset(&t, 0, sizeof(t));

		col = 0;

		str_to_number(t.dwVnum, data[col++]);
		strlcpy(t.szName, data[col++], sizeof(t.szName));
		str_to_number(t.bType, data[col++]);
		str_to_number(t.bMaxLevel, data[col++]);
		str_to_number(t.dwSplashRange, data[col++]);

		strlcpy(t.szPointOn, data[col++], sizeof(t.szPointOn));
		strlcpy(t.szPointPoly, data[col++], sizeof(t.szPointPoly));
		strlcpy(t.szSPCostPoly, data[col++], sizeof(t.szSPCostPoly));
		strlcpy(t.szDurationPoly, data[col++], sizeof(t.szDurationPoly));
		strlcpy(t.szDurationSPCostPoly, data[col++], sizeof(t.szDurationSPCostPoly));
		strlcpy(t.szCooldownPoly, data[col++], sizeof(t.szCooldownPoly));
		strlcpy(t.szMasterBonusPoly, data[col++], sizeof(t.szMasterBonusPoly));

		str_to_number(t.dwFlag, data[col++]);
		str_to_number(t.dwAffectFlag, data[col++]);

		strlcpy(t.szPointOn2, data[col++], sizeof(t.szPointOn2));
		strlcpy(t.szPointPoly2, data[col++], sizeof(t.szPointPoly2));
		strlcpy(t.szDurationPoly2, data[col++], sizeof(t.szDurationPoly2));
		str_to_number(t.dwAffectFlag2, data[col++]);

		strlcpy(t.szPointOn3, data[col++], sizeof(t.szPointOn3));
		strlcpy(t.szPointPoly3, data[col++], sizeof(t.szPointPoly3));
		strlcpy(t.szDurationPoly3, data[col++], sizeof(t.szDurationPoly3));

		strlcpy(t.szGrandMasterAddSPCostPoly, data[col++], sizeof(t.szGrandMasterAddSPCostPoly));

		str_to_number(t.bLevelStep, data[col++]);
		str_to_number(t.bLevelLimit, data[col++]);
		str_to_number(t.preSkillVnum, data[col++]);
		str_to_number(t.preSkillLevel, data[col++]);

		str_to_number(t.lMaxHit, data[col++]);

		strlcpy(t.szSplashAroundDamageAdjustPoly, data[col++], sizeof(t.szSplashAroundDamageAdjustPoly));

		str_to_number(t.bSkillAttrType, data[col++]);
		str_to_number(t.dwTargetRange, data[col++]);

		sys_log(0, "SKILL: #%d %s flag %u point %s affect %u cooldown %s", t.dwVnum, t.szName, t.dwFlag, t.szPointOn, t.dwAffectFlag, t.szCooldownPoly);

		m_vec_skillTable.push_back(t);
	}

	return true;
}

bool CClientManager::InitializeBanwordTable()
{
	m_vec_banwordTable.clear();

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery("SELECT word FROM banword"));

	SQLResult * pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
		return true;

	MYSQL_ROW data;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		TBanwordTable t;

		if (data[0])
		{
			strlcpy(t.szWord, data[0], sizeof(t.szWord));
			m_vec_banwordTable.push_back(t);
		}
	}

	sys_log(0, "BANWORD: total %d", m_vec_banwordTable.size());
	return true;
}

bool CClientManager::InitializeItemAttrTable()
{
	char query[4096];
	snprintf(query, sizeof(query),
			"SELECT apply, apply+0, prob, lv1, lv2, lv3, lv4, lv5, weapon, body, wrist, foots, neck, head, shield, ear"
#ifdef __PENDANT__
			", pendant"
#endif
			" FROM item_attr%s ORDER BY apply",
			GetTablePostfix());

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult * pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
	{
		sys_err("no result from item_attr");
		return false;
	}

	if (!m_vec_itemAttrTable.empty())
	{
		sys_log(0, "RELOAD: item_attr");
		m_vec_itemAttrTable.clear();
	}

	m_vec_itemAttrTable.reserve(pRes->uiNumRows);

	MYSQL_ROW	data;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		TItemAttrTable t;

		memset(&t, 0, sizeof(TItemAttrTable));

		int col = 0;

		strlcpy(t.szApply, data[col++], sizeof(t.szApply));
		str_to_number(t.dwApplyIndex, data[col++]);
		str_to_number(t.dwProb, data[col++]);
		str_to_number(t.lValues[0], data[col++]);
		str_to_number(t.lValues[1], data[col++]);
		str_to_number(t.lValues[2], data[col++]);
		str_to_number(t.lValues[3], data[col++]);
		str_to_number(t.lValues[4], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_WEAPON], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_BODY], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_WRIST], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_FOOTS], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_NECK], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_HEAD], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_SHIELD], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_EAR], data[col++]);
#ifdef __PENDANT__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_PENDANT], data[col++]);
#endif
		sys_log(0, "ITEM_ATTR: %-20s %4lu { %3d %3d %3d %3d %3d } { %d %d %d %d %d %d %d"
#ifdef __PENDANT__
				" %d"
#endif
				" }",
				t.szApply,
				t.dwProb,
				t.lValues[0],
				t.lValues[1],
				t.lValues[2],
				t.lValues[3],
				t.lValues[4],
				t.bMaxLevelBySet[ATTRIBUTE_SET_WEAPON],
				t.bMaxLevelBySet[ATTRIBUTE_SET_BODY],
				t.bMaxLevelBySet[ATTRIBUTE_SET_WRIST],
				t.bMaxLevelBySet[ATTRIBUTE_SET_FOOTS],
				t.bMaxLevelBySet[ATTRIBUTE_SET_NECK],
				t.bMaxLevelBySet[ATTRIBUTE_SET_HEAD],
				t.bMaxLevelBySet[ATTRIBUTE_SET_SHIELD],
				t.bMaxLevelBySet[ATTRIBUTE_SET_EAR]
#ifdef __PENDANT__
				, t.bMaxLevelBySet[ATTRIBUTE_SET_PENDANT]
#endif
				);

		m_vec_itemAttrTable.push_back(t);
	}

	return true;
}

bool CClientManager::InitializeItemRareTable()
{
	char query[4096];
	snprintf(query, sizeof(query),
			"SELECT apply, apply+0, prob, lv1, lv2, lv3, lv4, lv5, weapon, body, wrist, foots, neck, head, shield, ear"
#ifdef __PENDANT__
			", pendant"
#endif
			" FROM item_attr_rare%s ORDER BY apply",
			GetTablePostfix());

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult * pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
	{
		sys_err("no result from item_attr_rare");
		return false;
	}

	if (!m_vec_itemRareTable.empty())
	{
		sys_log(0, "RELOAD: item_attr_rare");
		m_vec_itemRareTable.clear();
	}

	m_vec_itemRareTable.reserve(pRes->uiNumRows);

	MYSQL_ROW	data;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		TItemAttrTable t;

		memset(&t, 0, sizeof(TItemAttrTable));

		int col = 0;

		strlcpy(t.szApply, data[col++], sizeof(t.szApply));
		str_to_number(t.dwApplyIndex, data[col++]);
		str_to_number(t.dwProb, data[col++]);
		str_to_number(t.lValues[0], data[col++]);
		str_to_number(t.lValues[1], data[col++]);
		str_to_number(t.lValues[2], data[col++]);
		str_to_number(t.lValues[3], data[col++]);
		str_to_number(t.lValues[4], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_WEAPON], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_BODY], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_WRIST], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_FOOTS], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_NECK], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_HEAD], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_SHIELD], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_EAR], data[col++]);
#ifdef __PENDANT__
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_PENDANT], data[col++]);
#endif

		sys_log(0, "ITEM_RARE: %-20s %4lu { %3d %3d %3d %3d %3d } { %d %d %d %d %d %d %d"
#ifdef __PENDANT__
				" %d"
#endif
				" }",
				t.szApply,
				t.dwProb,
				t.lValues[0],
				t.lValues[1],
				t.lValues[2],
				t.lValues[3],
				t.lValues[4],
				t.bMaxLevelBySet[ATTRIBUTE_SET_WEAPON],
				t.bMaxLevelBySet[ATTRIBUTE_SET_BODY],
				t.bMaxLevelBySet[ATTRIBUTE_SET_WRIST],
				t.bMaxLevelBySet[ATTRIBUTE_SET_FOOTS],
				t.bMaxLevelBySet[ATTRIBUTE_SET_NECK],
				t.bMaxLevelBySet[ATTRIBUTE_SET_HEAD],
				t.bMaxLevelBySet[ATTRIBUTE_SET_SHIELD],
				t.bMaxLevelBySet[ATTRIBUTE_SET_EAR]
#ifdef __PENDANT__
				, t.bMaxLevelBySet[ATTRIBUTE_SET_PENDANT]
#endif
				);

		m_vec_itemRareTable.push_back(t);
	}

	return true;
}

bool CClientManager::InitializeLandTable()
{
	using namespace building;

	char query[4096];

	snprintf(query, sizeof(query),
		"SELECT id, map_index, x, y, width, height, guild_id, guild_level_limit, price "
		"FROM land%s WHERE enable='YES' ORDER BY id",
		GetTablePostfix());

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult * pRes = pkMsg->Get();

	if (!m_vec_kLandTable.empty())
	{
		sys_log(0, "RELOAD: land");
		m_vec_kLandTable.clear();
	}

	m_vec_kLandTable.reserve(pRes->uiNumRows);

	MYSQL_ROW	data;

	if (pRes->uiNumRows > 0)
		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			TLand t;

			memset(&t, 0, sizeof(t));

			int col = 0;

			str_to_number(t.dwID, data[col++]);
			str_to_number(t.lMapIndex, data[col++]);
			str_to_number(t.x, data[col++]);
			str_to_number(t.y, data[col++]);
			str_to_number(t.width, data[col++]);
			str_to_number(t.height, data[col++]);
			str_to_number(t.dwGuildID, data[col++]);
			str_to_number(t.bGuildLevelLimit, data[col++]);
			str_to_number(t.dwPrice, data[col++]);

			sys_log(0, "LAND: %lu map %-4ld %7ldx%-7ld w %-4ld h %-4ld", t.dwID, t.lMapIndex, t.x, t.y, t.width, t.height);

			m_vec_kLandTable.push_back(t);
		}

	return true;
}

void parse_pair_number_string(const char * c_pszString, std::vector<std::pair<int, int> > & vec)
{
	const char * t = c_pszString;
	const char * p = strchr(t, '/');
	std::pair<int, int> k;

	char szNum[32 + 1];
	char * comma;

	while (p)
	{
		if (isnhdigit(*t))
		{
			strlcpy(szNum, t, MIN(sizeof(szNum), (p-t)+1));

			comma = strchr(szNum, ',');

			if (comma)
			{
				*comma = '\0';
				str_to_number(k.second, comma+1);
			}
			else
				k.second = 0;

			str_to_number(k.first, szNum);
			vec.push_back(k);
		}

		t = p + 1;
		p = strchr(t, '/');
	}

	if (isnhdigit(*t))
	{
		strlcpy(szNum, t, sizeof(szNum));

		comma = strchr(const_cast<char*>(t), ',');

		if (comma)
		{
			*comma = '\0';
			str_to_number(k.second, comma+1);
		}
		else
			k.second = 0;

		str_to_number(k.first, szNum);
		vec.push_back(k);
	}
}

bool CClientManager::InitializeObjectProto()
{
	using namespace building;

	char query[4096];
	snprintf(query, sizeof(query),
			"SELECT vnum, price, materials, upgrade_vnum, upgrade_limit_time, life, reg_1, reg_2, reg_3, reg_4, npc, group_vnum, dependent_group "
			"FROM object_proto%s ORDER BY vnum",
			GetTablePostfix());

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult * pRes = pkMsg->Get();

	if (!m_vec_kObjectProto.empty())
	{
		sys_log(0, "RELOAD: object_proto");
		m_vec_kObjectProto.clear();
	}

	m_vec_kObjectProto.reserve(MAX(0, pRes->uiNumRows));

	MYSQL_ROW	data;

	if (pRes->uiNumRows > 0)
		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			TObjectProto t;

			memset(&t, 0, sizeof(t));

			int col = 0;

			str_to_number(t.dwVnum, data[col++]);
			str_to_number(t.dwPrice, data[col++]);

			std::vector<std::pair<int, int> > vec;
			parse_pair_number_string(data[col++], vec);

			for (unsigned int i = 0; i < OBJECT_MATERIAL_MAX_NUM && i < vec.size(); ++i)
			{
				std::pair<int, int> & r = vec[i];

				t.kMaterials[i].dwItemVnum = r.first;
				t.kMaterials[i].dwCount = r.second;
			}

			str_to_number(t.dwUpgradeVnum, data[col++]);
			str_to_number(t.dwUpgradeLimitTime, data[col++]);
			str_to_number(t.lLife, data[col++]);
			str_to_number(t.lRegion[0], data[col++]);
			str_to_number(t.lRegion[1], data[col++]);
			str_to_number(t.lRegion[2], data[col++]);
			str_to_number(t.lRegion[3], data[col++]);

			str_to_number(t.dwNPCVnum, data[col++]);
			str_to_number(t.dwGroupVnum, data[col++]);
			str_to_number(t.dwDependOnGroupVnum, data[col++]);

			t.lNPCX = 0;
			t.lNPCY = MAX(t.lRegion[1], t.lRegion[3])+300;

			sys_log(0, "OBJ_PROTO: vnum %lu price %lu mat %lu %lu",
					t.dwVnum, t.dwPrice, t.kMaterials[0].dwItemVnum, t.kMaterials[0].dwCount);

			m_vec_kObjectProto.push_back(t);
		}

	return true;
}

bool CClientManager::InitializeObjectTable()
{
	using namespace building;

	char query[4096];
	snprintf(query, sizeof(query), "SELECT id, land_id, vnum, map_index, x, y, x_rot, y_rot, z_rot, life FROM object%s ORDER BY id", GetTablePostfix());

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
	SQLResult * pRes = pkMsg->Get();

	if (!m_map_pkObjectTable.empty())
	{
		sys_log(0, "RELOAD: object");
		m_map_pkObjectTable.clear();
	}

	MYSQL_ROW data;

	if (pRes->uiNumRows > 0)
		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			TObject * k = new TObject;

			memset(k, 0, sizeof(TObject));

			int col = 0;

			str_to_number(k->dwID, data[col++]);
			str_to_number(k->dwLandID, data[col++]);
			str_to_number(k->dwVnum, data[col++]);
			str_to_number(k->lMapIndex, data[col++]);
			str_to_number(k->x, data[col++]);
			str_to_number(k->y, data[col++]);
			str_to_number(k->xRot, data[col++]);
			str_to_number(k->yRot, data[col++]);
			str_to_number(k->zRot, data[col++]);
			str_to_number(k->lLife, data[col++]);

			sys_log(0, "OBJ: %lu vnum %lu map %-4ld %7ldx%-7ld life %ld",
					k->dwID, k->dwVnum, k->lMapIndex, k->x, k->y, k->lLife);

			m_map_pkObjectTable.insert(std::make_pair(k->dwID, k));
		}

	return true;
}

bool CClientManager::MirrorMobTableIntoDB()
{
	for (itertype(m_vec_mobTable) it = m_vec_mobTable.begin(); it != m_vec_mobTable.end(); it++)
	{
		const TMobTable& t = *it;
		char query[4096];
		snprintf(query, sizeof(query),
			"replace into mob_proto%s "
			"("
			"vnum, name, locale_name, type, rank, battle_type, level, scale, size, ai_flag, setRaceFlag, setImmuneFlag, "	//12
			"on_click, empire, drop_item, resurrection_vnum, folder, "	// 17
			"st, dx, ht, iq, damage_min, damage_max, max_hp, regen_cycle, regen_percent, exp, "	// 27
			"gold_min, gold_max, def, attack_speed, move_speed, aggressive_hp_pct, aggressive_sight, attack_range, polymorph_item, "	//36

			"enchant_curse, enchant_slow, enchant_poison, enchant_stun, enchant_critical, enchant_penetrate, "	//42
			"resist_sword, resist_twohand, resist_dagger, resist_bell, resist_fan, resist_bow, resist_claw, "	//49
			"resist_fire, resist_elect, resist_magic, resist_wind, resist_ice, resist_earth, resist_dark, resist_poison, resist_bleeding, "	//58
			"resist_fist, att_elect, att_fire, att_ice, att_wind, att_earth, att_dark, "	//65
			"dam_multiply, summon, drain_sp, "	//68

			"skill_vnum0, skill_level0, skill_vnum1, skill_level1, skill_vnum2, skill_level2, "	//74
			"skill_vnum3, skill_level3, skill_vnum4, skill_level4, "	// 78
			"sp_berserk, sp_stoneskin, sp_godspeed, sp_deathblow, sp_revive, "	//83
			"heal_vnum, hit_range"	//85
			") "
			"values ("

			"%u, \"%s\", \"%s\", %u, %u, %u, %u, %u, %u, %u, %u, %u, "	//12
			"%u, %u, %u, %u, '%s', "	//17
			"%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, "	//27
			"%u, %u, %d, %d, %d, %u, %d, %d, %u, "	//36

			"%d, %d, %d, %d, %d, %d, "	//42
			"%d, %d, %d, %d, %d, %d, %d, "	//49
			"%d, %d, %d, %d, %d, %d, %d, %d, %d, "	//58
			"%d, %d, %d, %d, %d, %d, %d, "	//65
			"%f, %u, %u, "	//68

			"%u, %u, %u, %u, %u, %u, "	//74
			"%u, %u, %u, %u, "	//78
			"%u, %u, %u, %u, %u, "	//83
			"%d, %d"	//85
			")",
			GetTablePostfix(),

			t.dwVnum, t.szName, t.szLocaleName, t.bType, t.bRank, t.bBattleType, t.bLevel, t.bScalePct, t.bSize, t.dwAIFlag, t.dwRaceFlag, t.dwImmuneFlag,	//12
			t.bOnClickType, t.bEmpire, t.dwDropItemVnum, t.dwResurrectionVnum, t.szFolder,	//17
			t.bStr, t.bDex, t.bCon, t.bInt, t.dwDamageRange[0], t.dwDamageRange[1], t.dwMaxHP, t.bRegenCycle, t.bRegenPercent, t.dwExp,	//27

			t.dwGoldMin, t.dwGoldMax, t.wDef, t.sAttackSpeed, t.sMovingSpeed, t.bAggresiveHPPct, t.wAggressiveSight, t.wAttackRange, t.dwPolymorphItemVnum,	//36
	
			t.cEnchants[0], t.cEnchants[1], t.cEnchants[2], t.cEnchants[3], t.cEnchants[4], t.cEnchants[5],	//42
			t.cResists[0], t.cResists[1], t.cResists[2], t.cResists[3], t.cResists[4], t.cResists[5], t.cResists[6],	//49
			t.cResists[7], t.cResists[8], t.cResists[9], t.cResists[10], t.cResists[11], t.cResists[12], t.cResists[13], t.cResists[14], t.cResists[15],	//58
			t.cResists[16], t.cAttrs[0], t.cAttrs[1], t.cAttrs[2], t.cAttrs[3], t.cAttrs[4], t.cAttrs[5],	//65
			t.fDamMultiply, t.dwSummonVnum, t.dwDrainSP,	//68

			t.Skills[0].dwVnum, t.Skills[0].bLevel, t.Skills[1].dwVnum, t.Skills[1].bLevel, t.Skills[2].dwVnum, t.Skills[2].bLevel,	//74
			t.Skills[3].dwVnum, t.Skills[3].bLevel, t.Skills[4].dwVnum, t.Skills[4].bLevel,	//78
			t.bBerserkPoint, t.bStoneSkinPoint, t.bGodSpeedPoint, t.bDeathBlowPoint, t.bRevivePoint,	//83
			t.dwHealerPoint, t.dwHitRange	//85
		);

		CDBManager::instance().AsyncQuery(query);
	}
	return true;
}

bool CClientManager::MirrorItemTableIntoDB()
{
	for (itertype(m_vec_itemTable) it = m_vec_itemTable.begin(); it != m_vec_itemTable.end(); it++)
	{
		const TItemTable& t = *it;
		char query[4096];
		snprintf(query, sizeof(query),
			"replace into item_proto%s (" //1
			"vnum, type, subtype, masktype, masksubtype, name, locale_name, gold, shop_buy_price, weight, size, " //2
			"flag, wearflag, antiflag, immuneflag, "
			"refined_vnum, refine_set, magic_pct, socket_pct, addon_type, "
			"limittype0, limitvalue0, limittype1, limitvalue1, "
			"applytype0, applyvalue0, applytype1, applyvalue1, applytype2, applyvalue2, applytype3, applyvalue3, "
			"value0, value1, value2, value3, value4, value5 ) "
			"values ("
			"%u, %u, %u, %u, %u, \"%s\", \"%s\", %u, %u, %u, %u, " //13
			"%u, %u, %u, %u, " //17
			"%u, %d, %u, %u, %d, " //22
			"%u, %ld, %u, %ld, " //26
			"%u, %ld, %u, %ld, %u, %ld, %u, %ld, " //34
			"%ld, %ld, %ld, %ld, %ld, %ld )", //40
			GetTablePostfix(), //1
			t.dwVnum, t.bType, t.bSubType, t.bMaskType, t.bMaskSubType, t.szName, t.szLocaleName, t.dwGold, t.dwShopBuyPrice, t.bWeight, t.bSize, //13
			t.dwFlags, t.dwWearFlags, t.dwAntiFlags, t.dwImmuneFlag, //17
			t.dwRefinedVnum, t.wRefineSet, t.bAlterToMagicItemPct, t.bGainSocketPct, t.sAddonType, //22
			t.aLimits[0].bType, t.aLimits[0].lValue, t.aLimits[1].bType, t.aLimits[1].lValue, //26
			t.aApplies[0].bType, t.aApplies[0].lValue, t.aApplies[1].bType, t.aApplies[1].lValue, t.aApplies[2].bType, t.aApplies[2].lValue, t.aApplies[3].bType, t.aApplies[3].lValue, //34
			t.alValues[0], t.alValues[1], t.alValues[2], t.alValues[3], t.alValues[4], t.alValues[5]); //40
		CDBManager::instance().AsyncQuery(query);
	}
	return true;
}

