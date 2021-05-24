#pragma once

#define WIN32_LEAN_AND_MEAN
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define _WIN32_DCOM

#include <d3d9/d3dx9.h>
#include <d3d9/dxErr.h>

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>

#include <mmsystem.h>
#include <process.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <direct.h>
#include <malloc.h>

#include "../EterBase/StdAfx.h"
#include "../EterBase/Debug.h"
#include "../EterBase/CodePageId.h"
#include "../UserInterface/Locale_inc.h"
#ifndef VC_EXTRALEAN
#include <winsock.h>
#endif
