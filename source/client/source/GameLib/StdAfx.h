#ifndef __INC_YMIR_GAMELIB__
#define __INC_YMIR_GAMELIB__
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#pragma warning(disable:4710)
#pragma warning(disable:4786)
#pragma warning(disable:4244)

#pragma warning(disable:4018)
#pragma warning(disable:4245)
#pragma warning(disable:4512)
#pragma warning(disable:4201)

#if _MSC_VER >= 1400
#pragma warning(disable:4201 4512 4238 4239)
#endif

#include "../EterBase/Utils.h"
#include "../EterBase/CRC32.h"
#include "../EterBase/Random.h"

#include "../EterLib/StdAfx.h"
#include "../MilesLib/StdAfx.h"
#include "../EffectLib/StdAfx.h"

#include "GameType.h"
#include "GameUtil.h"
#include "MapType.h"
#include "MapUtil.h"
#include "Interface.h"
#endif