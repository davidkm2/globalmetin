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
#pragma comment(lib, "lzo2.lib")

#include "Singleton.h"

#include "../../../source/client/eterGameLib/StdAfx.h"
#include "../../../source/client/eterGameLib/ItemData.h"

#include "../../../source/client/eterXClient/StdAfx.h"
#include "../../../source/client/eterXClient/PythonNonPlayer.h"

#define strncpy(a,b,c) strncpy_s(a,c,b,_TRUNCATE)
#define _snprintf(a,b,c,...) _snprintf_s(a,b,_TRUNCATE,c,__VA_ARGS__)

#include <algorithm>

//윈도우exe파일 만들면서 새로 추가 : 파일 읽어올 수 있도록 하였다.
#include "CsvFile.h"
#include "ItemCSVReader.h"

#define ENABLE_ADDONTYPE_AUTODETECT
