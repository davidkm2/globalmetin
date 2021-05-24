#include "ItemCSVReader.h"
#include <math.h>

bool bDebugFlag = false;
void writeDebug(const char* szMsg, int iInput, int type_value)
{
	static char __buf[512];
	static FILE * f1 = NULL;
	if (NULL == f1)
		fopen_s(&f1, "debug.log", "a+");

	if (type_value)
		_snprintf(__buf, sizeof(__buf), "ERROR: (%d->%d) %s\n", type_value, iInput, szMsg);
	else
		_snprintf(__buf, sizeof(__buf), "ERROR: (%d) %s\n", iInput, szMsg);
	fwrite(__buf, sizeof(char), strlen(__buf), f1);
}

using namespace std;

inline string trim_left(const string& str)
{
    string::size_type n = str.find_first_not_of(" \t\v\n\r");
    return n == string::npos ? str : str.substr(n, str.length());
}

inline string trim_right(const string& str)
{
    string::size_type n = str.find_last_not_of(" \t\v\n\r");
    return n == string::npos ? str : str.substr(0, n + 1);
}

string trim(const string& str){return trim_left(trim_right(str));}

static string* StringSplit(string strOrigin, string strTok)
{
    int     cutAt;                            //자르는위치
    int     index     = 0;                    //문자열인덱스
    string* strResult = new string[30];		  //결과return 할변수

    //strTok을찾을때까지반복
    while ((cutAt = strOrigin.find_first_of(strTok)) != strOrigin.npos)
    {
       if (cutAt > 0)  //자르는위치가0보다크면(성공시)
       {
            strResult[index++] = strOrigin.substr(0, cutAt);  //결과배열에추가
       }
       strOrigin = strOrigin.substr(cutAt+1);  //원본은자른부분제외한나머지
    }

    if(strOrigin.length() > 0)  //원본이아직남았으면
    {
        strResult[index++] = strOrigin.substr(0, cutAt);  //나머지를결과배열에추가
    }

	for( int i=0;i<index;i++)
	{
		strResult[i] = trim(strResult[i]);
	}

    return strResult;  //결과return
}


string arITVType[] = {
	"ITEM_NONE",
	"ITEM_WEAPON",
	"ITEM_ARMOR",
	"ITEM_USE",
	"ITEM_AUTOUSE",
	"ITEM_MATERIAL",
	"ITEM_SPECIAL",
	"ITEM_TOOL",
	"ITEM_LOTTERY",
	"ITEM_ELK",

	"ITEM_METIN",
	"ITEM_CONTAINER",
	"ITEM_FISH", 
	"ITEM_ROD",
	"ITEM_RESOURCE",
	"ITEM_CAMPFIRE",
	"ITEM_UNIQUE",
	"ITEM_SKILLBOOK",
	"ITEM_QUEST", 
	"ITEM_POLYMORPH",

	"ITEM_TREASURE_BOX",
	"ITEM_TREASURE_KEY",
	"ITEM_SKILLFORGET",
	"ITEM_GIFTBOX",
	"ITEM_PICK", 
	"ITEM_HAIR",
	"ITEM_TOTEM", 
	"ITEM_BLEND",
	"ITEM_COSTUME",
	"ITEM_DS",

	"ITEM_SPECIAL_DS",	
	"ITEM_EXTRACT",
	"ITEM_SECONDARY_COIN",

	"ITEM_RING",
	"ITEM_BELT", // 34

	"ITEM_TYPE1",
	"ITEM_TYPE2",
	"ITEM_TYPE3",
	"ITEM_TYPE4", //
	"ITEM_TYPE4",
	"ITEM_TYPE5",
	"ITEM_TYPE6",
	"ITEM_TYPE7",
	"ITEM_TYPE8"
};

int get_Item_Type_Value(string inputString)
{
	int retInt = -1;
	//cout << "Type : " << typeStr << " -> ";
	for (int j=0;j<sizeof(arITVType)/sizeof(arITVType[0]);j++) {
		string tempString = arITVType[j];
		if	(inputString.find(tempString)!=string::npos && tempString.find(inputString)!=string::npos) {
			//cout << j << " ";
			retInt =  j;
			break;
		}
	}
	//cout << endl;

	return retInt;

}

string set_Item_Type_Value(int iInput) // enum -1=None
{
	if (iInput==(BYTE)-1)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arITVType)/sizeof(arITVType[0]);i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDebugFlag)
				iInput = -1;
			inputString = arITVType[i];
			break;
		}
	}
	if (bDebugFlag && iInput!=-1)
		writeDebug("Item Type not found", iInput);
		
	return inputString;
}


string arIAFVSub1[] = {
	"WEAPON_SWORD",
	"WEAPON_DAGGER",
	"WEAPON_BOW",
	"WEAPON_TWO_HANDED",
	"WEAPON_BELL",
	"WEAPON_FAN",
	"WEAPON_ARROW",
	"WEAPON_MOUNT_SPEAR",
	"WEAPON_1",
	"WEAPON_2",
	"WEAPON_3", //
	"WEAPON_4",
	"WEAPON_5",
	"WEAPON_6",
	"WEAPON_7",
	"WEAPON_8",
	"WEAPON_9",
	"WEAPON_10",
	"WEAPON_11"
};

string arIAFVSub2[] = {
	"ARMOR_BODY",
	"ARMOR_HEAD",
	"ARMOR_SHIELD",
	"ARMOR_WRIST",
	"ARMOR_FOOTS",
	"ARMOR_NECK",
	"ARMOR_EAR",
	"ARMOR_PENDANT",
	"ARMOR_NUM_TYPES", //
	"ARMOR_NUM_TYPES2",
	"ARMOR_NUM_TYPES3",
	"ARMOR_NUM_TYPES4",
	"ARMOR_NUM_TYPES5",
	"ARMOR_NUM_TYPES6",
	"ARMOR_NUM_TYPES7",
	"ARMOR_NUM_TYPES8",
	"ARMOR_NUM_TYPES9",
	"ARMOR_NUM_TYPES10",
	"ARMOR_NUM_TYPES11"
};

string arIAFVSub3[] = {
	"USE_POTION",
	"USE_TALISMAN",
	"USE_TUNING",
	"USE_MOVE",
	"USE_TREASURE_BOX",
	"USE_MONEYBAG",
	"USE_BAIT",
	"USE_ABILITY_UP",
	"USE_AFFECT",
	"USE_CREATE_STONE",
	"USE_SPECIAL",
	"USE_POTION_NODELAY",
	"USE_CLEAR",
	"USE_INVISIBILITY",
	"USE_DETACHMENT",
	"USE_BUCKET",
	"USE_POTION_CONTINUE",
	"USE_CLEAN_SOCKET",
	"USE_CHANGE_ATTRIBUTE",
	"USE_ADD_ATTRIBUTE",
	"USE_ADD_ACCESSORY_SOCKET",
	"USE_PUT_INTO_ACCESSORY_SOCKET",
	"USE_ADD_ATTRIBUTE2",
	"USE_RECIPE",
	"USE_CHANGE_ATTRIBUTE2",
	"USE_BIND",
	"USE_UNBIND",
	"USE_TIME_CHARGE_PER",
	"USE_TIME_CHARGE_FIX", 
	"USE_PUT_INTO_BELT_SOCKET",
	"USE_PUT_INTO_RING_SOCKET",
	"USE_1",
	"USE_2",
	"USE_3",
	"USE_4",
	"USE_5", //
	"USE_6",
	"USE_7",
	"USE_8",
	"USE_9",
	"USE_10",
	"USE_11",
	"USE_12",
	"USE_13",
	"USE_14"
};

string arIAFVSub4[] = {
	"AUTOUSE_POTION",
	"AUTOUSE_ABILITY_UP",
	"AUTOUSE_BOMB",
	"AUTOUSE_GOLD",
	"AUTOUSE_MONEYBAG",
	"AUTOUSE_TREASURE_BOX", //
	"AUTOUSE_TREASURE_BOX1",
	"AUTOUSE_TREASURE_BOX2",
	"AUTOUSE_TREASURE_BOX3",
	"AUTOUSE_TREASURE_BOX4",
	"AUTOUSE_TREASURE_BOX5",
	"AUTOUSE_TREASURE_BOX6",
	"AUTOUSE_TREASURE_BOX7",
	"AUTOUSE_TREASURE_BOX8",
	"AUTOUSE_TREASURE_BOX9"
};

string arIAFVSub5[] = {
	"MATERIAL_LEATHER",
	"MATERIAL_BLOOD",
	"MATERIAL_ROOT",
	"MATERIAL_NEEDLE",
	"MATERIAL_JEWEL",
	"MATERIAL_DS_REFINE_NORMAL",
	"MATERIAL_DS_REFINE_BLESSED",
	"MATERIAL_DS_REFINE_HOLLY", //
	"MATERIAL_DS_REFINE_HOLLY1",
	"MATERIAL_DS_REFINE_HOLLY2",
	"MATERIAL_DS_REFINE_HOLLY3",
	"MATERIAL_DS_REFINE_HOLLY4",
	"MATERIAL_DS_REFINE_HOLLY5",
	"MATERIAL_DS_REFINE_HOLLY6",
	"MATERIAL_DS_REFINE_HOLLY7",
	"MATERIAL_DS_REFINE_HOLLY8",
	"MATERIAL_DS_REFINE_HOLLY9",
	"MATERIAL_DS_REFINE_HOLLY10"
};

string arIAFVSub6[] = {
	"SPECIAL_MAP",
	"SPECIAL_KEY",
	"SPECIAL_DOC",
	"SPECIAL_SPIRIT", //
	"SPECIAL_SPIRIT1",
	"SPECIAL_SPIRIT2",
	"SPECIAL_SPIRIT3",
	"SPECIAL_SPIRIT4",
	"SPECIAL_SPIRIT5",
	"SPECIAL_SPIRIT6",
	"SPECIAL_SPIRIT7",
	"SPECIAL_SPIRIT8",
	"SPECIAL_SPIRIT9",
	"SPECIAL_SPIRIT10"
};

string arIAFVSub7[] = {
	"TOOL_FISHING_ROD", //
	"TOOL_FISHING_ROD1",
	"TOOL_FISHING_ROD2",
	"TOOL_FISHING_ROD3",
	"TOOL_FISHING_ROD4",
	"TOOL_FISHING_ROD5",
	"TOOL_FISHING_ROD6",
	"TOOL_FISHING_ROD7",
	"TOOL_FISHING_ROD8",
	"TOOL_FISHING_ROD9",
	"TOOL_FISHING_ROD10"
};

string arIAFVSub8[] = {
	"LOTTERY_TICKET",
	"LOTTERY_INSTANT", //
	"LOTTERY_INSTANT1",
	"LOTTERY_INSTANT2",
	"LOTTERY_INSTANT3",
	"LOTTERY_INSTANT4",
	"LOTTERY_INSTANT5",
	"LOTTERY_INSTANT6",
	"LOTTERY_INSTANT7",
	"LOTTERY_INSTANT8",
	"LOTTERY_INSTANT9"
};

string arIAFVSub9[] = {
	"SUBTYPE9_1",
	"SUBTYPE9_2",
	"SUBTYPE9_3",
	"SUBTYPE9_4",
	"SUBTYPE9_5",
	"SUBTYPE9_6",
	"SUBTYPE9_7",
	"SUBTYPE9_8",
	"SUBTYPE9_9",
	"SUBTYPE9_10",
	"SUBTYPE9_12",
	"SUBTYPE9_13",
	"SUBTYPE9_14",
	"SUBTYPE9_15",
	"SUBTYPE9_16",
	"SUBTYPE9_17"
};

string arIAFVSub10[] = {
	"METIN_NORMAL",
	"METIN_GOLD", //
	"METIN_GOLD1",
	"METIN_GOLD2",
	"METIN_GOLD3",
	"METIN_GOLD4",
	"METIN_GOLD5",
	"METIN_GOLD6",
	"METIN_GOLD7",
	"METIN_GOLD8",
	"METIN_GOLD9"
};

string arIAFVSub11[] = {
	"SUBTYPE11_1", //
	"SUBTYPE11_2",
	"SUBTYPE11_3",
	"SUBTYPE11_4",
	"SUBTYPE11_5",
	"SUBTYPE11_6",
	"SUBTYPE11_7",
	"SUBTYPE11_8",
	"SUBTYPE11_9",
	"SUBTYPE11_10",
	"SUBTYPE11_12",
	"SUBTYPE11_13",
	"SUBTYPE11_14",
	"SUBTYPE11_15",
	"SUBTYPE11_16",
	"SUBTYPE11_17"
};

string arIAFVSub12[] = {
	"FISH_ALIVE",
	"FISH_DEAD", //
	"FISH_DEAD1",
	"FISH_DEAD2",
	"FISH_DEAD3",
	"FISH_DEAD4",
	"FISH_DEAD5",
	"FISH_DEAD6",
	"FISH_DEAD7",
	"FISH_DEAD8",
	"FISH_DEAD9"
};

string arIAFVSub13[] = {
	"SUBTYPE13_1", //
	"SUBTYPE13_2",
	"SUBTYPE13_3",
	"SUBTYPE13_4",
	"SUBTYPE13_5",
	"SUBTYPE13_6",
	"SUBTYPE13_7",
	"SUBTYPE13_8",
	"SUBTYPE13_9",
	"SUBTYPE13_10",
	"SUBTYPE13_12",
	"SUBTYPE13_13",
	"SUBTYPE13_14",
	"SUBTYPE13_15",
	"SUBTYPE13_16",
	"SUBTYPE13_17"
};

string arIAFVSub14[] = {
	"RESOURCE_FISHBONE",
	"RESOURCE_WATERSTONEPIECE",
	"RESOURCE_WATERSTONE",
	"RESOURCE_BLOOD_PEARL",
	"RESOURCE_BLUE_PEARL",
	"RESOURCE_WHITE_PEARL",
	"RESOURCE_BUCKET",
	"RESOURCE_CRYSTAL",
	"RESOURCE_GEM",
	"RESOURCE_STONE",
	"RESOURCE_METIN",
	"RESOURCE_ORE", //
	"RESOURCE_ORE1",
	"RESOURCE_ORE2",
	"RESOURCE_ORE3",
	"RESOURCE_ORE4",
	"RESOURCE_ORE5",
	"RESOURCE_ORE6",
	"RESOURCE_ORE7",
	"RESOURCE_ORE8",
	"RESOURCE_ORE9",
	"RESOURCE_ORE10"
};

string arIAFVSub15[] = {
	"SUBTYPE15_1", //
	"SUBTYPE15_2",
	"SUBTYPE15_3",
	"SUBTYPE15_4",
	"SUBTYPE15_5",
	"SUBTYPE15_6",
	"SUBTYPE15_7",
	"SUBTYPE15_8",
	"SUBTYPE15_9",
	"SUBTYPE15_10",
	"SUBTYPE15_12",
	"SUBTYPE15_13",
	"SUBTYPE15_14",
	"SUBTYPE15_15",
	"SUBTYPE15_16",
	"SUBTYPE15_17"
};

string arIAFVSub16[] = {
	"UNIQUE_NONE",
	"UNIQUE_BOOK",
	"UNIQUE_SPECIAL_RIDE",
	"UNIQUE_3",
	"UNIQUE_4",
	"UNIQUE_5",
	"UNIQUE_6",
	"UNIQUE_7",
	"UNIQUE_8",
	"UNIQUE_9",
	"USE_SPECIAL", //
	"USE_SPECIAL1",
	"USE_SPECIAL2",
	"USE_SPECIAL3",
	"USE_SPECIAL4",
	"USE_SPECIAL5",
	"USE_SPECIAL6",
	"USE_SPECIAL7",
	"USE_SPECIAL8",
	"USE_SPECIAL9",
	"USE_SPECIAL10"
};


string arIAFVSub17[] = {
	"SUBTYPE17_1", //
	"SUBTYPE17_2",
	"SUBTYPE17_3",
	"SUBTYPE17_4",
	"SUBTYPE17_5",
	"SUBTYPE17_6",
	"SUBTYPE17_7",
	"SUBTYPE17_8",
	"SUBTYPE17_9",
	"SUBTYPE17_10",
	"SUBTYPE17_12",
	"SUBTYPE17_13",
	"SUBTYPE17_14",
	"SUBTYPE17_15",
	"SUBTYPE17_16",
	"SUBTYPE17_17"
};

string arIAFVSub18[] = {
	"SUBTYPE18_1",
	"SUBTYPE18_2", //
	"SUBTYPE18_3",
	"SUBTYPE18_4",
	"SUBTYPE18_5",
	"SUBTYPE18_6",
	"SUBTYPE18_7",
	"SUBTYPE18_8",
	"SUBTYPE18_9",
	"SUBTYPE18_10",
	"SUBTYPE18_12",
	"SUBTYPE18_13",
	"SUBTYPE18_14",
	"SUBTYPE18_15",
	"SUBTYPE18_16",
	"SUBTYPE18_17"
};

string arIAFVSub19[] = {
	"SUBTYPE19_1", //
	"SUBTYPE19_2",
	"SUBTYPE19_3",
	"SUBTYPE19_4",
	"SUBTYPE19_5",
	"SUBTYPE19_6",
	"SUBTYPE19_7",
	"SUBTYPE19_8",
	"SUBTYPE19_9",
	"SUBTYPE19_10",
	"SUBTYPE19_12",
	"SUBTYPE19_13",
	"SUBTYPE19_14",
	"SUBTYPE19_15",
	"SUBTYPE19_16",
	"SUBTYPE19_17"
};

string arIAFVSub20[] = {
	"SUBTYPE20_1", //
	"SUBTYPE20_2",
	"SUBTYPE20_3",
	"SUBTYPE20_4",
	"SUBTYPE20_5",
	"SUBTYPE20_6",
	"SUBTYPE20_7",
	"SUBTYPE20_8",
	"SUBTYPE20_9",
	"SUBTYPE20_10",
	"SUBTYPE20_12",
	"SUBTYPE20_13",
	"SUBTYPE20_14",
	"SUBTYPE20_15",
	"SUBTYPE20_16",
	"SUBTYPE20_17"
};

string arIAFVSub21[] = {
	"SUBTYPE21_1", //
	"SUBTYPE21_2",
	"SUBTYPE21_3",
	"SUBTYPE21_4",
	"SUBTYPE21_5",
	"SUBTYPE21_6",
	"SUBTYPE21_7",
	"SUBTYPE21_8",
	"SUBTYPE21_9",
	"SUBTYPE21_10",
	"SUBTYPE21_12",
	"SUBTYPE21_13",
	"SUBTYPE21_14",
	"SUBTYPE21_15",
	"SUBTYPE21_16",
	"SUBTYPE21_17"
};

string arIAFVSub22[] = {
	"SUBTYPE22_1", //
	"SUBTYPE22_2",
	"SUBTYPE22_3",
	"SUBTYPE22_4",
	"SUBTYPE22_5",
	"SUBTYPE22_6",
	"SUBTYPE22_7",
	"SUBTYPE22_8",
	"SUBTYPE22_9",
	"SUBTYPE22_10",
	"SUBTYPE22_12",
	"SUBTYPE22_13",
	"SUBTYPE22_14",
	"SUBTYPE22_15",
	"SUBTYPE22_16",
	"SUBTYPE22_17"
};

string arIAFVSub23[] = {
	"SUBTYPE23_1", //
	"SUBTYPE23_2",
	"SUBTYPE23_3",
	"SUBTYPE23_4",
	"SUBTYPE23_5",
	"SUBTYPE23_6",
	"SUBTYPE23_7",
	"SUBTYPE23_8",
	"SUBTYPE23_9",
	"SUBTYPE23_10",
	"SUBTYPE23_12",
	"SUBTYPE23_13",
	"SUBTYPE23_14",
	"SUBTYPE23_15",
	"SUBTYPE23_16",
	"SUBTYPE23_17"
};

string arIAFVSub24[] = {
	"SUBTYPE24_1", //
	"SUBTYPE24_2",
	"SUBTYPE24_3",
	"SUBTYPE24_4",
	"SUBTYPE24_5",
	"SUBTYPE24_6",
	"SUBTYPE24_7",
	"SUBTYPE24_8",
	"SUBTYPE24_9",
	"SUBTYPE24_10",
	"SUBTYPE24_12",
	"SUBTYPE24_13",
	"SUBTYPE24_14",
	"SUBTYPE24_15",
	"SUBTYPE24_16",
	"SUBTYPE24_17"
};

string arIAFVSub25[] = {
	"SUBTYPE25_1",
	"SUBTYPE25_2",
	"SUBTYPE25_3",
	"SUBTYPE25_4",
	"SUBTYPE25_5",
	"SUBTYPE25_6",
	"SUBTYPE25_7",
	"SUBTYPE25_8",
	"SUBTYPE25_9",
	"SUBTYPE25_10",
	"SUBTYPE25_12",
	"SUBTYPE25_13",
	"SUBTYPE25_14",
	"SUBTYPE25_15",
	"SUBTYPE25_16",
	"SUBTYPE25_17"
};

string arIAFVSub26[] = {
	"SUBTYPE26_1",
	"SUBTYPE26_2",
	"SUBTYPE26_3",
	"SUBTYPE26_4",
	"SUBTYPE26_5",
	"SUBTYPE26_6",
	"SUBTYPE26_7",
	"SUBTYPE26_8",
	"SUBTYPE26_9",
	"SUBTYPE26_10",
	"SUBTYPE26_12",
	"SUBTYPE26_13",
	"SUBTYPE26_14",
	"SUBTYPE26_15",
	"SUBTYPE26_16",
	"SUBTYPE26_17"
};

string arIAFVSub27[] = {
	"SUBTYPE27_1", //
	"SUBTYPE27_2",
	"SUBTYPE27_3",
	"SUBTYPE27_4",
	"SUBTYPE27_5",
	"SUBTYPE27_6",
	"SUBTYPE27_7",
	"SUBTYPE27_8",
	"SUBTYPE27_9",
	"SUBTYPE27_10",
	"SUBTYPE27_12",
	"SUBTYPE27_13",
	"SUBTYPE27_14",
	"SUBTYPE27_15",
	"SUBTYPE27_16",
	"SUBTYPE27_17"
};

string arIAFVSub28[] = {
	"COSTUME_BODY",
	"COSTUME_HAIR",
	"COSTUME_HAIR0",
	"COSTUME_HAIR1",
	"COSTUME_HAIR2", //
	"COSTUME_HAIR3",
	"COSTUME_HAIR4",
	"COSTUME_HAIR5",
	"COSTUME_HAIR6",
	"COSTUME_HAIR7",
	"COSTUME_HAIR8",
	"COSTUME_HAIR9",
	"COSTUME_HAIR10"
};

string arIAFVSub29[] = {
	"DS_SLOT1",
	"DS_SLOT2",
	"DS_SLOT3",
	"DS_SLOT4",
	"DS_SLOT5",
	"DS_SLOT6", //
	"DS_SLOT61",
	"DS_SLOT62",
	"DS_SLOT63",
	"DS_SLOT64",
	"DS_SLOT65",
	"DS_SLOT66",
	"DS_SLOT67",
	"DS_SLOT68"
};

string arIAFVSub30[] = {
	"SUBTYPE30_1",
	"SUBTYPE30_2",
	"SUBTYPE30_3",
	"SUBTYPE30_4",
	"SUBTYPE30_5",
	"SUBTYPE30_6",
	"SUBTYPE30_7",
	"SUBTYPE30_8",
	"SUBTYPE30_9",
	"SUBTYPE30_10",
	"SUBTYPE30_12",
	"SUBTYPE30_13",
	"SUBTYPE30_14",
	"SUBTYPE30_15",
	"SUBTYPE30_16",
	"SUBTYPE30_17"
};

string arIAFVSub31[] = {
	"EXTRACT_DRAGON_SOUL",
	"EXTRACT_DRAGON_HEART", //
	"EXTRACT_DRAGON_HEART1",
	"EXTRACT_DRAGON_HEART2",
	"EXTRACT_DRAGON_HEART3",
	"EXTRACT_DRAGON_HEART4",
	"EXTRACT_DRAGON_HEART5",
	"EXTRACT_DRAGON_HEART6"
};

string arIAFVSub32[] = {
	"SUBTYPE32_1",
	"SUBTYPE32_2",
	"SUBTYPE32_3",
	"SUBTYPE32_4",
	"SUBTYPE32_5",
	"SUBTYPE32_6",
	"SUBTYPE32_7",
	"SUBTYPE32_8",
	"SUBTYPE32_9",
	"SUBTYPE32_10",
	"SUBTYPE32_12",
	"SUBTYPE32_13",
	"SUBTYPE32_14",
	"SUBTYPE32_15",
	"SUBTYPE32_16",
	"SUBTYPE32_17"
};

string arIAFVSub33[] = {
	"SUBTYPE33_1",
	"SUBTYPE33_2",
	"SUBTYPE33_3",
	"SUBTYPE33_4",
	"SUBTYPE33_5",
	"SUBTYPE33_6",
	"SUBTYPE33_7",
	"SUBTYPE33_8",
	"SUBTYPE33_9",
	"SUBTYPE33_10",
	"SUBTYPE33_12",
	"SUBTYPE33_13",
	"SUBTYPE33_14",
	"SUBTYPE33_15",
	"SUBTYPE33_16",
	"SUBTYPE33_17"
};

string arIAFVSub34[] = {
	"SUBTYPE34_1", //
	"SUBTYPE34_2",
	"SUBTYPE34_3",
	"SUBTYPE34_4",
	"SUBTYPE34_5",
	"SUBTYPE34_6",
	"SUBTYPE34_7",
	"SUBTYPE34_8",
	"SUBTYPE34_9",
	"SUBTYPE34_10",
	"SUBTYPE34_12",
	"SUBTYPE34_13",
	"SUBTYPE34_14",
	"SUBTYPE34_15",
	"SUBTYPE34_16",
	"SUBTYPE34_17"
};

string arIAFVSub35[] = {
	"PET_EGG",
	"PET_UPBRINGING",
	"PET_BAG",
	"PET_FEEDSTUFF",
	"PET_SKILL",
	"PET_SKILL_DEL_BOOK",
	"PET_NAME_CHANGE",
	"PET_EXPFOOD",
	"PET_SKILL_ALL_DEL_BOOK",
	"PET_EXPFOOD_PER",
	"PET_ATTR_CHANGE",
	"PET_ATTR_CHANGE_ITEMVNUM",
	"PET_PAY", //
	"SUBTYPE35_14",
	"SUBTYPE35_15"
};

string arIAFVSub36[] = {
	"SUBTYPE36_1",
	"SUBTYPE36_2", //
	"SUBTYPE36_3",
	"SUBTYPE36_4",
	"SUBTYPE36_5",
	"SUBTYPE36_5",
	"SUBTYPE36_6",
	"SUBTYPE36_7",
	"SUBTYPE36_8",
	"SUBTYPE36_9",
	"SUBTYPE36_10",
	"SUBTYPE36_11",
	"SUBTYPE36_12",
	"SUBTYPE36_13",
	"SUBTYPE36_14"
};

string arIAFVSub37[] = {
	"SUBTYPE37_1", //
	"SUBTYPE37_2",
	"SUBTYPE37_3",
	"SUBTYPE37_4",
	"SUBTYPE37_5",
	"SUBTYPE37_6",
	"SUBTYPE37_7",
	"SUBTYPE37_8",
	"SUBTYPE37_9",
	"SUBTYPE37_10",
	"SUBTYPE37_11",
	"SUBTYPE37_12",
	"SUBTYPE37_13",
	"SUBTYPE37_14",
	"SUBTYPE37_15"
};

string arIAFVSub38[] = {
	"SUBTYPE38_1",
	"SUBTYPE38_2", //
	"SUBTYPE38_3",
	"SUBTYPE38_4",
	"SUBTYPE38_5",
	"SUBTYPE38_6",
	"SUBTYPE38_7",
	"SUBTYPE38_8",
	"SUBTYPE38_9",
	"SUBTYPE38_10",
	"SUBTYPE38_12",
	"SUBTYPE38_13",
	"SUBTYPE38_14",
	"SUBTYPE38_15",
	"SUBTYPE38_16",
	"SUBTYPE38_17"
};

string arIAFVSub0[] = {
	"SUBTYPE0_1", //
	"SUBTYPE0_2",
	"SUBTYPE0_3",
	"SUBTYPE0_4",
	"SUBTYPE0_5",
	"SUBTYPE0_6",
	"SUBTYPE0_7",
	"SUBTYPE0_8",
	"SUBTYPE0_9",
	"SUBTYPE0_10",
	"SUBTYPE0_12",
	"SUBTYPE0_13",
	"SUBTYPE0_14",
	"SUBTYPE0_15",
	"SUBTYPE0_16",
	"SUBTYPE0_17"
};

string* arIAFVSubType[] = {
	arIAFVSub0,				//0
	arIAFVSub1,		//1
	arIAFVSub2,		//2
	arIAFVSub3,		//3
	arIAFVSub4,		//4
	arIAFVSub5,		//5
	arIAFVSub6,		//6
	arIAFVSub7,		//7
	arIAFVSub8,		//8
	arIAFVSub9,				//9
	arIAFVSub10,	//10
	arIAFVSub11,				//11
	arIAFVSub12,	//12
	arIAFVSub13,				//13
	arIAFVSub14,	//14
	arIAFVSub15,				//15
	arIAFVSub16,	//16
	arIAFVSub17,				//17
	arIAFVSub18,				//18
	arIAFVSub19,				//19
	arIAFVSub20,				//20
	arIAFVSub21,				//21
	arIAFVSub22,				//22
	arIAFVSub23,				//23
	arIAFVSub24,				//24
	arIAFVSub25,				//25
	arIAFVSub26,				//26
	arIAFVSub27,				//27
	arIAFVSub28,	//28
	arIAFVSub29,	//29
	arIAFVSub29,	//30
	arIAFVSub31,	//31
	arIAFVSub32,				//32
	arIAFVSub33,				//33
	arIAFVSub34,				//34
	arIAFVSub35,	//35
	arIAFVSub36,	//36
	arIAFVSub37,	//37
	arIAFVSub38,	//38
};

int arNumberOfSubtype[] =
{
	sizeof(arIAFVSub0)/sizeof(arIAFVSub0[0]),
	sizeof(arIAFVSub1)/sizeof(arIAFVSub1[0]),
	sizeof(arIAFVSub2)/sizeof(arIAFVSub2[0]),
	sizeof(arIAFVSub3)/sizeof(arIAFVSub3[0]),
	sizeof(arIAFVSub4)/sizeof(arIAFVSub4[0]),
	sizeof(arIAFVSub5)/sizeof(arIAFVSub5[0]),
	sizeof(arIAFVSub6)/sizeof(arIAFVSub6[0]),
	sizeof(arIAFVSub7)/sizeof(arIAFVSub7[0]),
	sizeof(arIAFVSub8)/sizeof(arIAFVSub8[0]),
	sizeof(arIAFVSub9)/sizeof(arIAFVSub9[0]),
	sizeof(arIAFVSub10)/sizeof(arIAFVSub10[0]),
	sizeof(arIAFVSub11)/sizeof(arIAFVSub11[0]),
	sizeof(arIAFVSub12)/sizeof(arIAFVSub12[0]),
	sizeof(arIAFVSub13)/sizeof(arIAFVSub13[0]),
	sizeof(arIAFVSub14)/sizeof(arIAFVSub14[0]),
	sizeof(arIAFVSub15)/sizeof(arIAFVSub15[0]),
	sizeof(arIAFVSub16)/sizeof(arIAFVSub16[0]),
	sizeof(arIAFVSub17)/sizeof(arIAFVSub17[0]),
	sizeof(arIAFVSub18)/sizeof(arIAFVSub18[0]),
	sizeof(arIAFVSub19)/sizeof(arIAFVSub19[0]),
	sizeof(arIAFVSub20)/sizeof(arIAFVSub20[0]),
	sizeof(arIAFVSub21)/sizeof(arIAFVSub21[0]),
	sizeof(arIAFVSub22)/sizeof(arIAFVSub22[0]),
	sizeof(arIAFVSub23)/sizeof(arIAFVSub23[0]),
	sizeof(arIAFVSub24)/sizeof(arIAFVSub24[0]),
	sizeof(arIAFVSub25)/sizeof(arIAFVSub25[0]),
	sizeof(arIAFVSub26)/sizeof(arIAFVSub26[0]),
	sizeof(arIAFVSub27)/sizeof(arIAFVSub27[0]),
	sizeof(arIAFVSub28)/sizeof(arIAFVSub28[0]),
	sizeof(arIAFVSub29)/sizeof(arIAFVSub29[0]),
	sizeof(arIAFVSub30)/sizeof(arIAFVSub30[0]),
	sizeof(arIAFVSub31)/sizeof(arIAFVSub31[0]),
	sizeof(arIAFVSub32)/sizeof(arIAFVSub32[0]),
	sizeof(arIAFVSub33)/sizeof(arIAFVSub33[0]),
	sizeof(arIAFVSub34)/sizeof(arIAFVSub34[0]), //34
	sizeof(arIAFVSub35)/sizeof(arIAFVSub35[0]),
	sizeof(arIAFVSub36)/sizeof(arIAFVSub36[0]),
	sizeof(arIAFVSub37)/sizeof(arIAFVSub37[0]),
	sizeof(arIAFVSub38)/sizeof(arIAFVSub38[0]),
};

int get_Item_SubType_Value(int type_value, string inputString)
{
	//아이템 타입의 서브타입 어레이가 존재하는지 알아보고, 없으면 0 리턴
	if (arIAFVSubType[type_value]==0) {
		return 0;
	}
	//

	int retInt = -1;
	//cout << "SubType : " << subTypeStr << " -> ";
	for (int j=0;j<arNumberOfSubtype[type_value];j++) {
		string tempString = arIAFVSubType[type_value][j];
		string tempInputString = trim(inputString);
		if	(tempInputString.compare(tempString)==0)
		{
			//cout << j << " ";
			retInt =  j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

string set_Item_SubType_Value(int type_value, int iInput) // enum -1=None
{
	// if (type_value==-1 || arIAFVSubType[type_value]==0 || iInput==-1)
	if (type_value==(BYTE)-1 || arIAFVSubType[type_value]==0 || iInput==(BYTE)-1)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<arNumberOfSubtype[type_value];i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDebugFlag)
				iInput = -1;
			inputString = arIAFVSubType[type_value][i];
			break;
		}
	}
	if (bDebugFlag && iInput!=-1)
		writeDebug("Item SubType not found", iInput, type_value);
	return inputString;
}

string arIMTVMaskType[] = {
	"MASK_ITEM_TYPE_NONE",
	"MASK_ITEM_TYPE_MOUNT_PET",
	"MASK_ITEM_TYPE_EQUIPMENT_WEAPON",
	"MASK_ITEM_TYPE_EQUIPMENT_ARMOR",
	"MASK_ITEM_TYPE_EQUIPMENT_JEWELRY",
	"MASK_ITEM_TYPE_TUNING",
	"MASK_ITEM_TYPE_POTION",
	"MASK_ITEM_TYPE_FISHING_PICK",
	"MASK_ITEM_TYPE_DRAGON_STONE",
	"MASK_ITEM_TYPE_COSTUMES",
	"MASK_ITEM_TYPE_SKILL",
	"MASK_ITEM_TYPE_UNIQUE",
	"MASK_ITEM_TYPE_ETC"
};

int get_Item_MaskType_Value(string inputString)
{
	int retInt = -1;
	for (int j = 0; j < sizeof(arIMTVMaskType) / sizeof(arIMTVMaskType[0]); j++)
	{
		string tempString = arIMTVMaskType[j];
		if (inputString.find(tempString) != string::npos && tempString.find(inputString) != string::npos)
		{
			retInt =  j;
			break;
		}
	}

	return retInt;
}

string set_Item_MaskType_Value(int iInput)
{
	if (iInput == (BYTE) - 1)
		return "0";

	string inputString("");
	int tmpFlag;
	for (int i = 0; i < sizeof(arIMTVMaskType) / sizeof(arIMTVMaskType[0]); i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDebugFlag)
				iInput = -1;
			inputString = arIMTVMaskType[i];
			break;
		}
	}
	if (bDebugFlag && iInput != -1)
		writeDebug("Item MaskType not found", iInput);
	return inputString;
}

string arIAFVMaskSub1[] = {
	"MASK_ITEM_SUBTYPE_MOUNT_PET_MOUNT",			//0
	"MASK_ITEM_SUBTYPE_MOUNT_PET_CHARGED_PET",		//1
	"MASK_ITEM_SUBTYPE_MOUNT_PET_FREE_PET",			//2
	"MASK_ITEM_SUBTYPE_MOUNT_PET_EGG"				//3
};

string arIAFVMaskSub2[] = {
	"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_SWORD",		//0
	"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_DAGGER",		//1
	"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_BOW",			//2
	"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_TWO_HANDED",	//3
	"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_BELL",			//4
	"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_CLAW",			//5
	"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_FAN",			//6
	"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_MOUNT_SPEAR",	//7
	"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_ARROW",		//8
	"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_QUIVER"		//9
};

string arIAFVMaskSub3[] = {
	"MASK_ITEM_SUBTYPE_ARMOR_ARMOR_BODY",			//0
	"MASK_ITEM_SUBTYPE_ARMOR_ARMOR_HEAD",			//1
	"MASK_ITEM_SUBTYPE_ARMOR_ARMOR_SHIELD"			//2
};

string arIAFVMaskSub4[] = {
	"MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_WRIST",		//0
	"MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_FOOTS",		//1
	"MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_NECK",			//2
	"MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_EAR",			//3
	"MASK_ITEM_SUBTYPE_JEWELRY_ITEM_BELT",			//4
	"MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_PENDANT"		//5
};

string arIAFVMaskSub5[] = {
	"MASK_ITEM_SUBTYPE_TUNING_RESOURCE",			//0
	"MASK_ITEM_SUBTYPE_TUNING_STONE",				//1
	"MASK_ITEM_SUBTYPE_TUNING_ETC"					//2
};

string arIAFVMaskSub6[] = {
	"MASK_ITEM_SUBTYPE_POTION_ABILITY",				//0
	"MASK_ITEM_SUBTYPE_POTION_HAIRDYE",				//1
	"MASK_ITEM_SUBTYPE_POTION_ETC"					//2
};

string arIAFVMaskSub7[] = {
	"MASK_ITEM_SUBTYPE_FISHING_PICK_FISHING_POLE",	//0
	"MASK_ITEM_SUBTYPE_FISHING_PICK_EQUIPMENT_PICK",//1
	"MASK_ITEM_SUBTYPE_FISHING_PICK_FOOD",			//2
	"MASK_ITEM_SUBTYPE_FISHING_PICK_STONE",			//3
	"MASK_ITEM_SUBTYPE_FISHING_PICK_ETC"			//4
};

string arIAFVMaskSub8[] = {
	"MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_DIAMOND",//0
	"MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_RUBY",	//1
	"MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_JADE",	//2
	"MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_SAPPHIRE",//3
	"MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_GARNET",	//4
	"MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_ONYX",	//5
	"MASK_ITEM_SUBTYPE_DRAGON_STONE_ETC"			//6
};

string arIAFVMaskSub9[] = {
	"MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_WEAPON",	//0
	"MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_BODY",		//1
	"MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_HAIR",		//2
	"MASK_ITEM_SUBTYPE_COSTUMES_SASH",				//3
	"MASK_ITEM_SUBTYPE_COSTUMES_ETC"				//4
};

string arIAFVMaskSub10[] = {
	"MASK_ITEM_SUBTYPE_SKILL_PAHAE",				//0
	"MASK_ITEM_SUBTYPE_SKILL_SKILL_BOOK",			//1
	"MASK_ITEM_SUBTYPE_SKILL_BOOK_OF_OBLIVION",		//2
	"MASK_ITEM_SUBTYPE_SKILL_ETC"					//3
};

string arIAFVMaskSub11[] = {
	"MASK_ITEM_SUBTYPE_UNIQUE_ABILITY",				//0
	"MASK_ITEM_SUBTYPE_UNIQUE_ETC"					//1
};

string arIAFVMaskSub12[] = {
	"MASK_ITEM_SUBTYPE_ETC_GIFTBOX",				//0
	"MASK_ITEM_SUBTYPE_ETC_MATRIMORY",				//1
	"MASK_ITEM_SUBTYPE_ETC_EVENT",					//2
	"MASK_ITEM_SUBTYPE_ETC_SEAL",					//3
	"MASK_ITEM_SUBTYPE_ETC_PARTI",					//4
	"MASK_ITEM_SUBTYPE_ETC_POLYMORPH",				//5
	"MASK_ITEM_SUBTYPE_ETC_RECIPE",					//6
	"MASK_ITEM_SUBTYPE_ETC_ETC"						//7
};

string* arIAFVMaskSubType[] = {
	0,					//0
	arIAFVMaskSub1,		//1
	arIAFVMaskSub2,		//2
	arIAFVMaskSub3,		//3
	arIAFVMaskSub4,		//4
	arIAFVMaskSub5,		//5
	arIAFVMaskSub6,		//6
	arIAFVMaskSub7,		//7
	arIAFVMaskSub8,		//8
	arIAFVMaskSub9,		//9
	arIAFVMaskSub10,	//10
	arIAFVMaskSub11,	//11
	arIAFVMaskSub12,	//12
};

int arNumberOfMaskSubtype[] =
{
	0,
	sizeof(arIAFVMaskSub1)/sizeof(arIAFVMaskSub1[0]),
	sizeof(arIAFVMaskSub2)/sizeof(arIAFVMaskSub2[0]),
	sizeof(arIAFVMaskSub3)/sizeof(arIAFVMaskSub3[0]),
	sizeof(arIAFVMaskSub4)/sizeof(arIAFVMaskSub4[0]),
	sizeof(arIAFVMaskSub5)/sizeof(arIAFVMaskSub5[0]),
	sizeof(arIAFVMaskSub6)/sizeof(arIAFVMaskSub6[0]),
	sizeof(arIAFVMaskSub7)/sizeof(arIAFVMaskSub7[0]),
	sizeof(arIAFVMaskSub8)/sizeof(arIAFVMaskSub8[0]),
	sizeof(arIAFVMaskSub9)/sizeof(arIAFVMaskSub9[0]),
	sizeof(arIAFVMaskSub10)/sizeof(arIAFVMaskSub10[0]),
	sizeof(arIAFVMaskSub11)/sizeof(arIAFVMaskSub11[0]),
	sizeof(arIAFVMaskSub12)/sizeof(arIAFVMaskSub12[0]),
};

int get_Item_MaskSubType_Value(int type_value, string inputString)
{
	if (arIAFVMaskSubType[type_value] == 0)
		return 0;

	int retInt = -1;
	for (int j = 0; j < arNumberOfMaskSubtype[type_value]; j++)
	{
		string tempString = arIAFVMaskSubType[type_value][j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			retInt =  j;
			break;
		}
	}

	return retInt;
}

string set_Item_MaskSubType_Value(int type_value, int iInput)
{
	if (type_value ==(BYTE) - 1 || arIAFVMaskSubType[type_value] == 0 || iInput == (BYTE) - 1)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i = 0;  i < arNumberOfMaskSubtype[type_value]; i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDebugFlag)
				iInput = -1;
			inputString = arIAFVMaskSubType[type_value][i];
			break;
		}
	}
	if (bDebugFlag && iInput!=-1)
		writeDebug("Item SubType not found", iInput, type_value);
	return inputString;
}

string arIAFVAntiFlag[] = {
	"ANTI_FEMALE",
	"ANTI_MALE",
	"ANTI_MUSA", 
	"ANTI_ASSASSIN",
	"ANTI_SURA",
	"ANTI_MUDANG",
	"ANTI_GET", 
	"ANTI_DROP",
	"ANTI_SELL",
	"ANTI_EMPIRE_A",
	"ANTI_EMPIRE_B",
	"ANTI_EMPIRE_C",
	"ANTI_SAVE", 
	"ANTI_GIVE", 
	"ANTI_PKDROP",
	"ANTI_STACK",
	"ANTI_MYSHOP",
	"ANTI_SAFEBOX",
	"ANTI_WOLFMAN",
	"ANTI_PET",
	"ANTI_QUICKSLOT",
	"ANTI_CHANGELOOK",
	"ANTI_REINFORCE",
	"ANTI_ENCHANT",
	"ANTI_ENERGY",
	"ANTI_PETFEED",
	"ANTI_APPLY",
	"ANTI_ACCE",
	"ANTI_MAIL", //
	"ANTI_SAFEBOX11",
	"ANTI_SAFEBOX12",
	"ANTI_SAFEBOX13",
	"ANTI_SAFEBOX14",
	"ANTI_SAFEBOX15",
	"ANTI_SAFEBOX16",
	"ANTI_SAFEBOX17",
	"ANTI_SAFEBOX18",
	"ANTI_SAFEBOX19",
	"ANTI_SAFEBOX20",
	"ANTI_SAFEBOX21",
	"ANTI_SAFEBOX22",
	"ANTI_SAFEBOX23",
	"ANTI_SAFEBOX24",
	"ANTI_SAFEBOX25",
	"ANTI_SAFEBOX26"
};

int get_Item_AntiFlag_Value(string inputString)
{
	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|");				//프로토 정보 내용을 단어별로 쪼갠 배열.
	for(int i =0;i<sizeof(arIAFVAntiFlag)/sizeof(arIAFVAntiFlag[0]);i++) {
		string tempString = arIAFVAntiFlag[i];
		for (int j=0; j<30 ; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString)==0) {				//일치하는지 확인.
				retValue = retValue + static_cast<int>(pow(2.0, static_cast<double>(i)));
			}

			if(tempString2.compare("") == 0)
				break;
		}
	}
	delete []arInputString;
	//cout << "AntiFlag : " << antiFlagStr << " -> " << retValue << endl;

	return retValue;
}

string set_Item_AntiFlag_Value(int iInput) // set
{
	if (iInput==0)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arIAFVAntiFlag)/sizeof(arIAFVAntiFlag[0]);i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += " | ";
			inputString += arIAFVAntiFlag[i];
		}
	}
	if (bDebugFlag && iInput)
		writeDebug("Item AntiFlag not found", iInput);
	return inputString;
}


string arIFVFlag[] = {
	"ITEM_TUNABLE", 
	"ITEM_SAVE", 
	"ITEM_STACKABLE",
	"COUNT_PER_1GOLD",
	"ITEM_SLOW_QUERY", 
	"ITEM_UNIQUE",
	"ITEM_MAKECOUNT", 
	"ITEM_IRREMOVABLE", 
	"CONFIRM_WHEN_USE", 
	"QUEST_USE", 
	"QUEST_USE_MULTIPLE",
	"QUEST_GIVE",
	// "ITEM_QUEST",
	"LOG",
	// "STACKABLE",
	// "SLOW_QUERY", 
	// "REFINEABLE",
	// "IRREMOVABLE", 
	"ITEM_APPLICABLE", //
	"ITEM_APPLICABLE1",
	"ITEM_APPLICABLE2",
	"ITEM_APPLICABLE3",
	"ITEM_APPLICABLE4"
};

int get_Item_Flag_Value(string inputString)
{
	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|");				//프로토 정보 내용을 단어별로 쪼갠 배열.
	for(int i =0;i<sizeof(arIFVFlag)/sizeof(arIFVFlag[0]);i++) {
		string tempString = arIFVFlag[i];
		for (int j=0; j<30 ; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString)==0) {				//일치하는지 확인.
				retValue = retValue + static_cast<int>(pow(2.0, static_cast<double>(i)));
			}

			if(tempString2.compare("") == 0)
				break;
		}
	}
	delete []arInputString;
	//cout << "Flag : " << flagStr << " -> " << retValue << endl;

	return retValue;
}

string set_Item_Flag_Value(int iInput) // set
{
	if (iInput==0)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arIFVFlag)/sizeof(arIFVFlag[0]);i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += " | ";
			inputString += arIFVFlag[i];
		}
	}
	if (bDebugFlag && iInput)
		writeDebug("Item Flag not found", iInput);
	return inputString;
}


string arIWFVWearrFlag[] = {
	"WEAR_BODY", 
	"WEAR_HEAD",
	"WEAR_FOOTS", 
	"WEAR_WRIST", 
	"WEAR_WEAPON", 
	"WEAR_NECK",
	"WEAR_EAR",
	"WEAR_SHIELD",
	"WEAR_UNIQUE",
	"WEAR_ARROW", 
	"WEAR_HAIR", 
	"WEAR_ABILITY",
	"WEAR_ABILITY0", //
	"WEAR_ABILITY1",
	"WEAR_ABILITY2",
	"WEAR_ABILITY3",
	"WEAR_ABILITY4",
	"WEAR_ABILITY5"
};

int get_Item_WearFlag_Value(string inputString)
{
	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|");				//프로토 정보 내용을 단어별로 쪼갠 배열.
	for(int i =0;i<sizeof(arIWFVWearrFlag)/sizeof(arIWFVWearrFlag[0]);i++) {
		string tempString = arIWFVWearrFlag[i];
		for (int j=0; j<30 ; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString)==0) {				//일치하는지 확인.
				retValue = retValue + static_cast<int>(pow(2.0, static_cast<double>(i)));
			}

			if(tempString2.compare("") == 0)
				break;
		}
	}
	delete []arInputString;
	//cout << "WearFlag : " << wearFlagStr << " -> " << retValue << endl;

	return retValue;
}

string set_Item_WearFlag_Value(int iInput) // set
{
	if (iInput==0)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arIWFVWearrFlag)/sizeof(arIWFVWearrFlag[0]);i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += " | ";
			inputString += arIWFVWearrFlag[i];
		}
	}
	if (bDebugFlag && iInput)
		writeDebug("Item WearFlag not found", iInput);
	return inputString;
}


string arIIVImmune[] = {
	"PARA",
	"CURSE",
	"STUN",
	"SLEEP",
	"SLOW",
	"POISON",
	"TERROR",//
	"TERROR2",
	"TERROR3",
	"TERROR4"
};

int get_Item_Immune_Value(string inputString)
{
	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|");				//프로토 정보 내용을 단어별로 쪼갠 배열.
	for(int i =0;i<sizeof(arIIVImmune)/sizeof(arIIVImmune[0]);i++) {
		string tempString = arIIVImmune[i];
		for (int j=0; j<30 ; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString)==0) {				//일치하는지 확인.
				retValue = retValue + static_cast<int>(pow(2.0, static_cast<double>(i)));
			}

			if(tempString2.compare("") == 0)
				break;
		}
	}
	delete []arInputString;
	//cout << "Immune : " << immuneStr << " -> " << retValue << endl;

	return retValue;
}

string set_Item_Immune_Value(int iInput) // set
{
	if (iInput==0)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arIIVImmune)/sizeof(arIIVImmune[0]);i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += " | ";
			inputString += arIIVImmune[i];
		}
	}
	if (bDebugFlag && iInput)
		writeDebug("Item ImmuneFlag not found", iInput);
	return inputString;
}


string arILTVLimitType[] = {
	"LIMIT_NONE",
	"LEVEL",
	"STR", 
	"DEX", 
	"INT", 
	"CON",
	"PC_BANG", 
	"REAL_TIME",
	"REAL_TIME_FIRST_USE",
	"TIMER_BASED_ON_WEAR",//
	"TIMER_BASED_ON_WEAR1",
	"TIMER_BASED_ON_WEAR2",
	"TIMER_BASED_ON_WEAR3"
};

int get_Item_LimitType_Value(string inputString)
{
	int retInt = -1;
	//cout << "LimitType : " << limitTypeStr << " -> ";
	for (int j=0;j<sizeof(arILTVLimitType)/sizeof(arILTVLimitType[0]);j++) {
		string tempString = arILTVLimitType[j];
		string tempInputString = trim(inputString);
		if	(tempInputString.compare(tempString)==0)
		{
			//cout << j << " ";
			retInt =  j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

string set_Item_LimitType_Value(int iInput) // enum -1=None
{
	if (iInput==(BYTE)-1)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arILTVLimitType)/sizeof(arILTVLimitType[0]);i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDebugFlag)
				iInput = -1;
			inputString = arILTVLimitType[i];
			break;
		}
	}
	if (bDebugFlag && iInput!=-1)
		writeDebug("Item LimitType not found", iInput);
	return inputString;
}


string arIATVApplyType[] = {
	"APPLY_NONE",
	"APPLY_MAX_HP",
	"APPLY_MAX_SP", 
	"APPLY_CON",
	"APPLY_INT", 
	"APPLY_STR", 
	"APPLY_DEX",
	"APPLY_ATT_SPEED",
	"APPLY_MOV_SPEED",
	"APPLY_CAST_SPEED", 
	"APPLY_HP_REGEN",
	"APPLY_SP_REGEN",
	"APPLY_POISON_PCT",
	"APPLY_STUN_PCT",
	"APPLY_SLOW_PCT",
	"APPLY_CRITICAL_PCT",
	"APPLY_PENETRATE_PCT",
	"APPLY_ATTBONUS_HUMAN", 
	"APPLY_ATTBONUS_ANIMAL",
	"APPLY_ATTBONUS_ORC",
	"APPLY_ATTBONUS_MILGYO", 
	"APPLY_ATTBONUS_UNDEAD", 
	"APPLY_ATTBONUS_DEVIL", 
	"APPLY_STEAL_HP",
	"APPLY_STEAL_SP", 
	"APPLY_MANA_BURN_PCT",
	"APPLY_DAMAGE_SP_RECOVER", 
	"APPLY_BLOCK",
	"APPLY_DODGE",
	"APPLY_RESIST_SWORD",
	"APPLY_RESIST_TWOHAND", 
	"APPLY_RESIST_DAGGER", 
	"APPLY_RESIST_BELL",
	"APPLY_RESIST_FAN", 
	"APPLY_RESIST_BOW",
	"APPLY_RESIST_FIRE",
	"APPLY_RESIST_ELEC",
	"APPLY_RESIST_MAGIC", 
	"APPLY_RESIST_WIND",
	"APPLY_REFLECT_MELEE", 
	"APPLY_REFLECT_CURSE", 
	"APPLY_POISON_REDUCE",
	"APPLY_KILL_SP_RECOVER",
	"APPLY_EXP_DOUBLE_BONUS",
	"APPLY_GOLD_DOUBLE_BONUS", 
	"APPLY_ITEM_DROP_BONUS", 
	"APPLY_POTION_BONUS",
	"APPLY_KILL_HP_RECOVER", 
	"APPLY_IMMUNE_STUN",
	"APPLY_IMMUNE_SLOW", 
	"APPLY_IMMUNE_FALL", 
	"APPLY_SKILL",
	"APPLY_BOW_DISTANCE",
	"APPLY_ATT_GRADE_BONUS",
	"APPLY_DEF_GRADE_BONUS", 
	"APPLY_MAGIC_ATT_GRADE", 
	"APPLY_MAGIC_DEF_GRADE",
	"APPLY_CURSE_PCT",
	"APPLY_MAX_STAMINA", 
	"APPLY_ATTBONUS_WARRIOR",
	"APPLY_ATTBONUS_ASSASSIN", 
	"APPLY_ATTBONUS_SURA",
	"APPLY_ATTBONUS_SHAMAN",
	"APPLY_ATTBONUS_MONSTER",
	"APPLY_MALL_ATTBONUS", 
	"APPLY_MALL_DEFBONUS", 
	"APPLY_MALL_EXPBONUS",
	"APPLY_MALL_ITEMBONUS",
	"APPLY_MALL_GOLDBONUS",
	"APPLY_MAX_HP_PCT", 
	"APPLY_MAX_SP_PCT",
	"APPLY_SKILL_DAMAGE_BONUS", 
	"APPLY_NORMAL_HIT_DAMAGE_BONUS",
	"APPLY_SKILL_DEFEND_BONUS",
	"APPLY_NORMAL_HIT_DEFEND_BONUS",
	"APPLY_PC_BANG_EXP_BONUS", 
	"APPLY_PC_BANG_DROP_BONUS",
	"APPLY_EXTRACT_HP_PCT",
	"APPLY_RESIST_WARRIOR",
	"APPLY_RESIST_ASSASSIN",
	"APPLY_RESIST_SURA",
	"APPLY_RESIST_SHAMAN",
	"APPLY_ENERGY",
	"APPLY_DEF_GRADE", 
	"APPLY_COSTUME_ATTR_BONUS",
	"APPLY_MAGIC_ATTBONUS_PER",
	"APPLY_MELEE_MAGIC_ATTBONUS_PER",
	"APPLY_RESIST_ICE",
	"APPLY_RESIST_EARTH", 
	"APPLY_RESIST_DARK", 
	"APPLY_ANTI_CRITICAL_PCT", 
	"APPLY_ANTI_PENETRATE_PCT",
	"APPLY_0",
	"APPLY_1",
	"APPLY_2",
	"APPLY_3",
	"APPLY_4",
	"APPLY_5",
	"APPLY_6",
	"APPLY_7",
	"APPLY_8",
	"APPLY_9",
	"APPLY_10",
	"APPLY_11",
	"APPLY_12",
	"APPLY_13",
	"APPLY_14",
	"APPLY_15",
	"APPLY_16",
	"APPLY_17",
	"APPLY_18",
	"APPLY_19",
	"APPLY_20",
	"APPLY_21",
	"APPLY_22",
	"APPLY_23",
	"APPLY_24",
	"APPLY_25",
	"APPLY_26", //
	"APPLY_27",
	"APPLY_28"
};

int get_Item_ApplyType_Value(string inputString)
{
	int retInt = -1;
	//cout << "ApplyType : " << applyTypeStr << " -> ";
	for (int j=0;j<sizeof(arIATVApplyType)/sizeof(arIATVApplyType[0]);j++) {
		string tempString = arIATVApplyType[j];
		string tempInputString = trim(inputString);
		if	(tempInputString.compare(tempString)==0)
		{
			//cout << j << " ";
			retInt =  j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

string set_Item_ApplyType_Value(int iInput) // enum -1=None
{
	// if (iInput==-1)
	if (iInput==(BYTE)-1)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arIATVApplyType)/sizeof(arIATVApplyType[0]);i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDebugFlag)
				iInput = -1;
			inputString = arIATVApplyType[i];
			break;
		}
	}
	if (bDebugFlag && iInput!=-1)
		writeDebug("Item ApplyType not found", iInput);
	return inputString;
}

/// MOB -----------------------------------

string arMRVRank[] = {
	"PAWN",
	"S_PAWN", 
	"KNIGHT", 
	"S_KNIGHT",
	"BOSS", 
	"KING", //
	"RANK0",
	"RANK1",
	"RANK2",
	"RANK3",
	"RANK4",
	"RANK5",
	"RANK6",
	"RANK7",
	"RANK8",
	"RANK9",
	"RANK10",
	"RANK11",
	"RANK12",
	"RANK13",
	"RANK14",
	"RANK15",
	"RANK16",
	"RANK17",
	"RANK18",
	"RANK19",
	"RANK20"
};

int get_Mob_Rank_Value(string inputString)
{

	int retInt = -1;
	//cout << "Rank : " << rankStr << " -> ";
	for (int j=0;j<sizeof(arMRVRank)/sizeof(arMRVRank[0]);j++) {
		string tempString = arMRVRank[j];
		string tempInputString = trim(inputString);
		if	(tempInputString.compare(tempString)==0)
		{
			//cout << j << " ";
			retInt =  j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

string set_Mob_Rank_Value(int iInput) // enum -1=None
{
	if (iInput==(BYTE)-1)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arMRVRank)/sizeof(arMRVRank[0]);i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDebugFlag)
				iInput = -1;
			inputString = arMRVRank[i];
			break;
		}
	}
	if (bDebugFlag && iInput!=-1)
		writeDebug("Mob Rank not found", iInput);
	return inputString;
}


string arMTVType[] = {
	"MONSTER",
	"NPC",
	"STONE",
	"WARP",
	"DOOR", 
	"BUILDING",
	"PC", 
	"POLYMORPH_PC",
	"HORSE",
	"GOTO",
	"NEW_PET",
	"PET", //
	"TYPE2",
	"TYPE3",
	"TYPE4",
	"TYPE5",
	"TYPE6",
	"TYPE7",
	"TYPE8",
	"TYPE9",
	"TYPE10",
	"TYPE11",
	"TYPE12",
	"TYPE13",
	"TYPE14",
	"TYPE15",
	"TYPE16",
	"TYPE17",
	"TYPE18",
	"TYPE19",
	"TYPE20"
};

int get_Mob_Type_Value(string inputString)
{
	int retInt = -1;
	//cout << "Type : " << typeStr << " -> ";
	for (int j=0;j<sizeof(arMTVType)/sizeof(arMTVType[0]);j++) {
		string tempString = arMTVType[j];
		string tempInputString = trim(inputString);
		if	(tempInputString.compare(tempString)==0)
		{
			//cout << j << " ";
			retInt =  j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

string set_Mob_Type_Value(int iInput) // enum -1=None
{
	if (iInput==(BYTE)-1)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arMTVType)/sizeof(arMTVType[0]);i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDebugFlag)
				iInput = -1;
			inputString = arMTVType[i];
			break;
		}
	}
	if (bDebugFlag && iInput!=-1)
		writeDebug("Mob Type not found", iInput);
	return inputString;
}


string arMBTVBattleType[] = {
	"MELEE",
	"RANGE",
	"MAGIC",
	"SPECIAL", 
	"POWER", 
	"TANKER",
	"SUPER_POWER", 
	"SUPER_TANKER", //
	"BATTLE_TYPE0",
	"BATTLE_TYPE1",
	"BATTLE_TYPE2",
	"BATTLE_TYPE3",
	"BATTLE_TYPE4",
	"BATTLE_TYPE5",
	"BATTLE_TYPE6",
	"BATTLE_TYPE7",
	"BATTLE_TYPE8",
	"BATTLE_TYPE9",
	"BATTLE_TYPE10",
	"BATTLE_TYPE11",
	"BATTLE_TYPE12",
	"BATTLE_TYPE13",
	"BATTLE_TYPE14",
	"BATTLE_TYPE15",
	"BATTLE_TYPE16",
	"BATTLE_TYPE17",
	"BATTLE_TYPE18",
	"BATTLE_TYPE19",
	"BATTLE_TYPE20"
};

int get_Mob_BattleType_Value(string inputString)
{
	int retInt = -1;
	//cout << "Battle Type : " << battleTypeStr << " -> ";
	for (int j=0;j<sizeof(arMBTVBattleType)/sizeof(arMBTVBattleType[0]);j++) {
		string tempString = arMBTVBattleType[j];
		string tempInputString = trim(inputString);
		if	(tempInputString.compare(tempString)==0)
		{
			//cout << j << " ";
			retInt =  j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

string set_Mob_BattleType_Value(int iInput) // enum -1=None
{
	if (iInput==(BYTE)-1)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arMBTVBattleType)/sizeof(arMBTVBattleType[0]);i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDebugFlag)
				iInput = -1;
			inputString = arMBTVBattleType[i];
			break;
		}
	}
	if (bDebugFlag && iInput!=-1)
		writeDebug("Mob BattleType not found", iInput);
	return inputString;
}


string arMSVSize[] = {
	"SMALL", 
	"MEDIUM", 
	"BIG", //
	"SIZE1",
	"SIZE2",
	"SIZE3",
	"SIZE4",
	"SIZE5",
	"SIZE6",
	"SIZE7",
	"SIZE8",
	"SIZE9",
	"SIZE10"
};

int get_Mob_Size_Value(string inputString)
{
	int retInt = 0;
	//cout << "Size : " << sizeStr << " -> ";
	for (int j=0;j<sizeof(arMSVSize)/sizeof(arMSVSize[0]);j++) {
		string tempString = arMSVSize[j];
		string tempInputString = trim(inputString);
		if	(tempInputString.compare(tempString)==0)
		{
			//cout << j << " ";
			retInt =  j + 1;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

string set_Mob_Size_Value(int iInput) // enum 0=None
{
	if (iInput==0)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arMSVSize)/sizeof(arMSVSize[0]);i++)
	{
		tmpFlag = i+1;
		if (iInput == tmpFlag)
		{
			if (bDebugFlag)
				iInput = 0;
			inputString = arMSVSize[i];
			break;
		}
	}
	if (bDebugFlag && iInput)
		writeDebug("Mob Size not found", iInput);
	return inputString;
}


string arMOBAIFVAIFlag[] = {
	"AGGR",
	"NOMOVE",
	"COWARD",
	"NOATTSHINSU",
	"NOATTCHUNJO",
	"NOATTJINNO",
	"ATTMOB",
	"BERSERK",
	"STONESKIN",
	"GODSPEED",
	"DEATHBLOW",
	"REVIVE",
	"HEALER"
	"COUNT",
	"NORECOVERY",
	"REFLECT",
	"FALL",
	"VIT",
	"RATTSPEED",
	"RCASTSPEED",
	"RHP_REGEN",
	"TIMEVIT",
	"AI_FLAG11", //
	"AI_FLAG12",
	"AI_FLAG13",
	"AI_FLAG14",
	"AI_FLAG15",
	"AI_FLAG16",
	"AI_FLAG17",
	"AI_FLAG18",
	"AI_FLAG19",
	"AI_FLAG20",
	"AI_FLAG21",
	"AI_FLAG22",
	"AI_FLAG23",
	"AI_FLAG24",
	"AI_FLAG25",
	"AI_FLAG26",
	"AI_FLAG27",
	"AI_FLAG28",
	"AI_FLAG29",
	"AI_FLAG30"
};

int get_Mob_AIFlag_Value(string inputString)
{
	int retValue = 0;
	string* arInputString = StringSplit(inputString, ",");				//프로토 정보 내용을 단어별로 쪼갠 배열.
	for(int i =0;i<sizeof(arMOBAIFVAIFlag)/sizeof(arMOBAIFVAIFlag[0]);i++) {
		string tempString = arMOBAIFVAIFlag[i];
		for (int j=0; j<30 ; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString)==0) {				//일치하는지 확인.
				retValue = retValue + static_cast<int>(pow(2.0, static_cast<double>(i)));
			}

			if(tempString2.compare("") == 0)
				break;
		}
	}
	delete []arInputString;
	//cout << "AIFlag : " << aiFlagStr << " -> " << retValue << endl;

	return retValue;
}

string set_Mob_AIFlag_Value(int iInput) // set
{
	if (iInput==0)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arMOBAIFVAIFlag)/sizeof(arMOBAIFVAIFlag[0]);i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += ",";
			inputString += arMOBAIFVAIFlag[i];
		}
	}
	if (bDebugFlag && iInput)
		writeDebug("Mob AIFlag not found", iInput);
	return inputString;
}


string arMRFVRaceFlag[] = {
	"ANIMAL",
	"UNDEAD",
	"DEVIL",
	"HUMAN",
	"ORC",
	"MILGYO",
	"INSECT",
	"FIRE",
	"ICE",
	"DESERT",
	"TREE",
	"ATT_ELEC",
	"ATT_FIRE",
	"ATT_ICE",
	"ATT_WIND",
	"ATT_EARTH",
	"ATT_DARK",
	"DECO",
	"HIDE",
	"ATT_CZ", //
	"RACE_FLAG4",
	"RACE_FLAG5",
	"RACE_FLAG6",
	"RACE_FLAG7",
	"RACE_FLAG8",
	"RACE_FLAG9",
	"RACE_FLAG10",
	"RACE_FLAG11",
	"RACE_FLAG12",
	"RACE_FLAG13",
	"RACE_FLAG14",
	"RACE_FLAG15",
	"RACE_FLAG16",
	"RACE_FLAG17",
	"RACE_FLAG18",
	"RACE_FLAG19",
	"RACE_FLAG20"
};

int get_Mob_RaceFlag_Value(string inputString)
{
	int retValue = 0;
	// string* arInputString = StringSplit(inputString, "|");				//프로토 정보 내용을 단어별로 쪼갠 배열.
	string* arInputString = StringSplit(inputString, ","); // @fixme201
	for(int i =0;i<sizeof(arMRFVRaceFlag)/sizeof(arMRFVRaceFlag[0]);i++) {
		string tempString = arMRFVRaceFlag[i];
		for (int j=0; j<30 ; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString)==0) {				//일치하는지 확인.
				retValue = retValue + static_cast<int>(pow(2.0, static_cast<double>(i)));
			}

			if(tempString2.compare("") == 0)
				break;
		}
	}
	delete []arInputString;
	//cout << "Race Flag : " << raceFlagStr << " -> " << retValue << endl;

	return retValue;
}

string set_Mob_RaceFlag_Value(int iInput) // set
{
	if (iInput==0)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arMRFVRaceFlag)/sizeof(arMRFVRaceFlag[0]);i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += ",";
			inputString += arMRFVRaceFlag[i];
		}
	}
	if (bDebugFlag && iInput)
		writeDebug("Mob RaceFlag not found", iInput);
	return inputString;
}


string arMIFVImmuneFlag[] = {
	"STUN",
	"SLOW",
	"FALL",
	"CURSE",
	"POISON",
	"TERROR", 
	"REFLECT",
	"IMMUNE_FLAG2",
	"IMMUNE_FLAG3",
	"IMMUNE_FLAG4",
	"IMMUNE_FLAG5",
	"IMMUNE_FLAG6",
	"IMMUNE_FLAG7",
	"IMMUNE_FLAG8", //
	"IMMUNE_FLAG9",
	"IMMUNE_FLAG10",
	"IMMUNE_FLAG11",
	"IMMUNE_FLAG12",
	"IMMUNE_FLAG13",
	"IMMUNE_FLAG14",
	"IMMUNE_FLAG15",
	"IMMUNE_FLAG16",
	"IMMUNE_FLAG17",
	"IMMUNE_FLAG18",
	"IMMUNE_FLAG19",
	"IMMUNE_FLAG20",
	"IMMUNE_FLAG21",
	"IMMUNE_FLAG22",
	"IMMUNE_FLAG23",
	"IMMUNE_FLAG24"
};

int get_Mob_ImmuneFlag_Value(string inputString)
{
	int retValue = 0;
	string* arInputString = StringSplit(inputString, ",");				//프로토 정보 내용을 단어별로 쪼갠 배열.
	for(int i =0;i<sizeof(arMIFVImmuneFlag)/sizeof(arMIFVImmuneFlag[0]);i++) {
		string tempString = arMIFVImmuneFlag[i];
		for (int j=0; j<30 ; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString)==0) {				//일치하는지 확인.
				retValue = retValue + static_cast<int>(pow(2.0, static_cast<double>(i)));
			}

			if(tempString2.compare("") == 0)
				break;
		}
	}
	delete []arInputString;
	//cout << "Immune Flag : " << immuneFlagStr << " -> " << retValue << endl;

	return retValue;
}

string set_Mob_ImmuneFlag_Value(int iInput) // set
{
	if (iInput==0)
		return "0";

	string inputString("");
	int tmpFlag;
	for(int i =0;i<sizeof(arMIFVImmuneFlag)/sizeof(arMIFVImmuneFlag[0]);i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += ",";
			inputString += arMIFVImmuneFlag[i];
		}
	}
	if (bDebugFlag && iInput)
		writeDebug("Mob ImmuneFlag not found", iInput);
	return inputString;
}

string set_Mob_Char(char szChar[])
{
	string asd(szChar);

	if (asd == "")
		return "0";
	else
		return asd;
}

string set_Mob_Dam(float fDamMultiply)
{
	string asd(to_string(fDamMultiply));

	return asd;
}

