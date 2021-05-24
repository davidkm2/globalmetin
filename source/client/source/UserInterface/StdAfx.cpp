#include "stdafx.h"
#include "locale_inc.h"

#ifdef CMAKE_BUILD_SYSTEM

#pragma comment( lib, "version.lib" )
#if (TARGET_ARCH == 32)
	#pragma comment( lib, "devil_x86.lib" )
#elif (TARGET_ARCH == 64)
	#pragma comment( lib, "devil_x64.lib" )
#else
	#pragma error ("unsupported architecture")
#endif
#pragma comment( lib, "mss32.lib" )
#pragma comment( lib, "imm32.lib" )
#pragma comment( lib, "dinput8.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "psapi.lib" )
#pragma comment( lib, "wtsapi32.lib" )
#pragma comment( lib, "wbemuuid.lib" )
#pragma comment( lib, "Netapi32.lib" )
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "dbghelp.lib" )
#pragma comment( lib, "granny2.11.8.0_static.lib" )
#pragma comment( lib, "CrashRpt1500.lib" )
#pragma comment( lib, "lzo2.lib" )
#pragma comment( lib, "detours.lib" )

#ifdef _DEBUG
	#pragma comment(lib, "python2-7-14_debug.lib")
	#pragma comment(lib, "tinyxml2d.lib")
	#pragma comment(lib, "xxhashd.lib")
	#pragma comment(lib, "cryptlib_debug.lib")
	#pragma comment(lib, "SpeedTreeRT_Static_d.lib")
#else
	#pragma comment(lib, "python2-7-14_relese.lib")
	#pragma comment(lib, "tinyxml2.lib")
	#pragma comment(lib, "xxhash.lib")
	#pragma comment(lib, "cryptlib_release.lib")
	#pragma comment(lib, "SpeedTreeRT_Static.lib")
#endif

#ifdef ENABLE_FOX_FS
	#pragma comment(lib, "lz4.lib")
#endif

#ifdef ENABLE_VFS
	#pragma comment(lib, "VFSLib.lib")
	#pragma comment(lib, "VFSCryptLib.lib")
#endif

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxerr.lib")


#else

#ifdef ENABLE_FOX_FS
#pragma comment(lib, "lz4.lib")
#endif

#pragma comment(lib, "version.lib")
#pragma comment(lib, "CrashRpt1500.lib")

#ifdef _DEBUG
#pragma comment(lib, "python27_d.lib")
#pragma comment( lib, "cryptopp_debug.lib" )
#pragma comment(lib, "tinyxml2d.lib")
#else
#pragma comment(lib, "python27.lib")
#pragma comment( lib, "cryptopp_release.lib" )
#pragma comment(lib, "tinyxml2.lib")
#endif

#pragma comment(lib, "DevIL.lib")
#pragma comment(lib, "lzo.lib")
#pragma comment(lib, "granny.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "mss32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "imm32.lib")
#pragma comment( lib, "psapi.lib" )
#pragma comment( lib, "wtsapi32.lib" )
#pragma comment( lib, "wbemuuid.lib" )
#pragma comment( lib, "Netapi32.lib" )
#pragma comment( lib, "dbghelp.lib" )

#pragma comment(lib, "SpeedTreeRT.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ulib.lib")
#endif
