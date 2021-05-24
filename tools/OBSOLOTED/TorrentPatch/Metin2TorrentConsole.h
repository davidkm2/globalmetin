// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#ifndef TORRENTPATCH_METIN2TORRENTCONSOLE_H_
#define TORRENTPATCH_METIN2TORRENTCONSOLE_H_

//#ifndef _WIN32_WINNT		// Windows XP �̻󿡼��� ����� ����� �� �ֽ��ϴ�.                   
//#define _WIN32_WINNT 0x0501	// �ٸ� ������ Windows�� �µ��� ������ ������ ������ �ֽʽÿ�.

// WIN98 ����
#define WINVER         0x0410
#define _WIN32_WINNT   0x0403 // Windows 2000
#define _WIN32_WINDOWS 0x0410 // WIN98
#define _WIN32_IE      0x0401 // ���ͳ� �ͽ��÷η� 4.1
//#endif						

// Windows ��� ����:
#include <winsock2.h>
#include <windows.h>

#include <wx-2.8.10/wxprec.h>
#ifndef WX_PRECOMP
   #include <wx-2.8.10/wx.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include <YmirBase/YmirBase.h>
#include <YmirBase/Primitives.h>
#include <YmirBase/Log.h>

#include <Earth/Earth.h>
#include <Earth/EA_CStatus.h>
#include <Mantle/MA_StringCharSet.h>
#include <Mantle/MA_StringNumeric.h>
#include <Mantle/MA_STLString.h>

#include <TinyXML-2.6.1/TinyXML.h>

//#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

#ifdef _DEBUG
#define TORRENT_DEBUG
#endif

#define _SFX_ENABLE

#define APP_VERSION_NAME			_T("v1.8.2")
#define APP_NAME					"YmirPatcher"
#define TAPP_NAME					_T("YmirPatcher")
#define TAPP_FULLNAME				_T("YmirPatcher") ## APP_VERSION_NAME
#define DIR_LOG						_T("temp")

#define FILENAME_PATCH_VER			_T("patch.ver")
#define FILENAME_PATCH_URL_XML		_T("TorrentPatch.url.xml")
#define FILENAME_APP_ICON			_T("TorrentPatch.ico")

#ifndef _DEBUG
#define UPDATE_PATCHER_SELF
#define USE_FAST_SELFUPDATE
#endif

#include "CLocaleManager.h"

#include "PatchInfo.h"
#include "PatchConfig.h"
#include "PatchStatus.h"

#endif  // TORRENTPATCH_METIN2TORRENTCONSOLE_H_
