#pragma once

#if USE_CODE_VIRTUALIZER_SDK == 1
#include <Protector/CodeVirtualizer/VirtualizerSDK.h>
#define __PROTECTOR__ "CodeVirtualizer"

//#define __PROTECTOR_START__(x)	VIRTUALIZER5_START
//#define __PROTECTOR_END__(x)		VIRTUALIZER5_END
#define __PROTECTOR_START__(x)		VIRTUALIZER_EAGLE_BLACK_START
#define __PROTECTOR_END__(x)		VIRTUALIZER_EAGLE_BLACK_END

#define __MUTATE_START__(x)			VIRTUALIZER_MUTATE_ONLY_START
#define __MUTATE_END__(x)			VIRTUALIZER_MUTATE_ONLY_END

#elif USE_SHIELDEN_SDK == 1
#include <Protector/Shielden/SESDK.h>
#define __PROTECTOR__ "Shielden"

#define __PROTECTOR_START__(x)	SE_PROTECT_START_VIRTUALIZATION
#define __MUTATE_START__(x)		SE_PROTECT_START_MUTATION

#define __PROTECTOR_END__(x)	SE_PROTECT_END
#define __MUTATE_END__(x)		SE_PROTECT_END

#elif USE_THEMIDA_SDK == 1
#include <Protector/Themida/ThemidaSDK.h>
#define __PROTECTOR__ "Themida"

#define __PROTECTOR_START__(x)	VM_START
#define __PROTECTOR_END__(x)	VM_END

#define __MUTATE_START__(x)		MUTATE_START
#define __MUTATE_END__(x)		MUTATE_END

#ifdef _WIN64
#pragma comment("Protector/Themida/SecureEngineSDK64.lib")
#else
#pragma comment("Protector/Themida/SecureEngineSDK32.lib")
#endif

#elif USE_VMPROTECT_SDK == 1
#include <Protector/VMProtect/VMProtectSDK.h>
#define __PROTECTOR__ "VMProtect"

#define __PROTECTOR_START__(x)	VMProtectBeginVirtualization(x);
#define __MUTATE_START__(x)		VMProtectBeginMutation(x);

#define __PROTECTOR_END__(x)	VMProtectEnd();
#define __MUTATE_END__(x)		VMProtectEnd();

#ifdef _WIN64
#pragma comment("VMProtect/VMProtectSDK64.lib")
#else
#pragma comment("VMProtect/VMProtectSDK32.lib")
#endif

#else

#define __PROTECTOR__ "None"

#define __PROTECTOR_START__(x)
#define __PROTECTOR_END__(x)

#define __MUTATE_START__(x)
#define __MUTATE_END__(x)

#endif

