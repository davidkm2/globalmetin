#pragma once
#define _USE_32BIT_TIME_T

#include <cstdio>
#include <ctime>

#include <iostream>
#include <string>

#include <set>
#include <map>
#include <list>

#include "lzo.h"
#pragma comment(lib, "lzo.lib")

#include "Singleton.h"

#include "../../../Client/source/GameLib/StdAfx.h"
#include "../../../Client/source/GameLib/ItemData.h"

#include "../../../Client/source/UserInterface/StdAfx.h"
#include "../../../Client/source/UserInterface/PythonNonPlayer.h"

#define strncpy(a,b,c) strncpy_s(a,c,b,_TRUNCATE)
#define _snprintf(a,b,c,...) _snprintf_s(a,b,_TRUNCATE,c,__VA_ARGS__)
inline bool operator<(const CItemData::TItemTable& lhs, const CItemData::TItemTable& rhs)
{
	return lhs.dwVnum < rhs.dwVnum;
}

#include <algorithm>

//윈도우exe파일 만들면서 새로 추가 : 파일 읽어올 수 있도록 하였다.
#include "CsvFile.h"
#include "ItemCSVReader.h"

#pragma comment(lib, "lzo.lib")

#define ENABLE_ADDONTYPE_AUTODETECT