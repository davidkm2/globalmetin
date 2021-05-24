#ifndef __Item_CSV_READER_H__
#define __Item_CSV_READER_H__

#include "dump_proto.h"
#include <iostream>
#include <string>
using std::string;

//csv 파일을 읽어와서 아이템 테이블에 넣어준다.
void putItemIntoTable(); //(테이블, 테스트여부)

int get_Item_Type_Value(std::string inputString);
int get_Item_SubType_Value(int type_value, std::string inputString);
int get_Item_MaskType_Value(string inputString);
int get_Item_MaskSubType_Value(int type_value, std::string inputString);
int get_Item_AntiFlag_Value(std::string inputString);
int get_Item_Flag_Value(std::string inputString);
int get_Item_WearFlag_Value(std::string inputString);
int get_Item_Immune_Value(std::string inputString);
int get_Item_LimitType_Value(std::string inputString);
int get_Item_ApplyType_Value(std::string inputString);

//몬스터 프로토도 읽을 수 있다.
int get_Mob_Rank_Value(std::string inputString);
int get_Mob_Type_Value(std::string inputString);
int get_Mob_BattleType_Value(std::string inputString);

int get_Mob_Size_Value(std::string inputString);
int get_Mob_AIFlag_Value(std::string inputString);
int get_Mob_RaceFlag_Value(std::string inputString);
int get_Mob_ImmuneFlag_Value(std::string inputString);


// ## DUMP
// # ITEM_PROTO
string set_Item_Type_Value(int iInput);
string set_Item_SubType_Value(int type_value, int iInput);
string set_Item_MaskType_Value(int iInput);
string set_Item_MaskSubType_Value(int type_value, int iInput);
string set_Item_AntiFlag_Value(int iInput);
string set_Item_Flag_Value(int iInput);
string set_Item_WearFlag_Value(int iInput);
string set_Item_Immune_Value(int iInput);
string set_Item_LimitType_Value(int iInput);
string set_Item_ApplyType_Value(int iInput);

// # MOB_PROTO
string set_Mob_Rank_Value(int iInput);
string set_Mob_Type_Value(int iInput);
string set_Mob_BattleType_Value(int iInput);
string set_Mob_Char(char szChar[]);
string set_Mob_Size_Value(int iInput);
string set_Mob_AIFlag_Value(int iInput);
string set_Mob_RaceFlag_Value(int iInput);
string set_Mob_ImmuneFlag_Value(int iInput);
string set_Mob_Dam(float fDamMultiply);

// ## DEBUG
extern bool bDebugFlag;
void writeDebug(const char* szMsg, int iInput, int type_value=0);

#endif

