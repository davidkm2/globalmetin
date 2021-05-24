#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"

#include <AccCtrl.h>
#include <Aclapi.h>
#include <sddl.h>

#include <XORstr.h>

#ifndef PROCESS_SET_LIMITED_INFORMATION
#define PROCESS_SET_LIMITED_INFORMATION 0x2000
#endif

#define THREAD_POISONED_ACCESS					\
	THREAD_SUSPEND_RESUME						| \
	THREAD_SET_CONTEXT							| \
	THREAD_SET_INFORMATION						| \
	THREAD_SET_THREAD_TOKEN						| \
	THREAD_TERMINATE

#define PROCESS_POISONED_ACCESS					\
	PROCESS_CREATE_THREAD						| \
	PROCESS_VM_WRITE							| \
	PROCESS_VM_READ								| \
	PROCESS_VM_OPERATION						| \
	PROCESS_SET_INFORMATION						| \
	PROCESS_SET_LIMITED_INFORMATION				| \
	PROCESS_QUERY_INFORMATION					| \
	PROCESS_SUSPEND_RESUME						| \
	PROCESS_CREATE_PROCESS

bool CAnticheatManager::BlockAccess(HANDLE hTarget)
{
	std::string szSD =
		XOR("D:P"				/* DACL */
			"(D;OICI;GA;;;BG)"  /* Deny access to built-in guests */
			"(D;OICI;GA;;;AN)"  /*		  ^		  anonymous logon */
			"(D;OICI;GA;;;AU)"  /*		  ^		  authenticated users */
			"(D;OICI;GA;;;BA)"  /*		  ^		  administrators */
			"(D;OICI;GA;;;LA)"  /*		  ^		  Built-in Administrator */
		);

	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE;

	if (!ConvertStringSecurityDescriptorToSecurityDescriptorA(szSD.c_str(), SDDL_REVISION_1, &(sa.lpSecurityDescriptor), nullptr))
	{
#ifdef _DEBUG
		TraceError("ConvertStringSecurityDescriptorToSecurityDescriptorA fail! Error: %u", GetLastError());
#endif
		return false;
	}

	if (!SetKernelObjectSecurity(hTarget, DACL_SECURITY_INFORMATION, sa.lpSecurityDescriptor))
	{
#ifdef _DEBUG
		TraceError("SetKernelObjectSecurity fail! Error: %u", GetLastError());
#endif
		LocalFree(sa.lpSecurityDescriptor);
		return false;
	}

	LocalFree(sa.lpSecurityDescriptor);
	return true;
}

bool CAnticheatManager::DenyProcessAccess()
{
	typedef NTSTATUS(NTAPI* TRtlAddAccessDeniedAce)(PACL pAcl, uint32_t dwAceRevision, ACCESS_MASK	AccessMask, PSID pSid);

	auto hNtdll = LoadLibraryA(XOR("ntdll.dll"));
	if (!hNtdll)
		return false;

	auto lpRtlAddAccessDeniedAce = GetProcAddress(hNtdll, XOR("RtlAddAccessDeniedAce"));
	if (!lpRtlAddAccessDeniedAce)
		return false;
	auto RtlAddAccessDeniedAce = (TRtlAddAccessDeniedAce)lpRtlAddAccessDeniedAce;

	SID pSid = { 0 };
	pSid.Revision = SID_REVISION;
	pSid.IdentifierAuthority = SECURITY_WORLD_SID_AUTHORITY;
	pSid.SubAuthorityCount = 1;
	pSid.SubAuthority[0] = SECURITY_WORLD_RID;

	uint8_t pBuffer[0x400] = { 0 };

	auto pACL = (PACL)&pBuffer;
	pACL->AclRevision = ACL_REVISION;
	pACL->AclSize = sizeof(pBuffer);
	pACL->AceCount = 0;

	auto ntStatus = RtlAddAccessDeniedAce(pACL, ACL_REVISION, PROCESS_POISONED_ACCESS, &pSid);
	if (!NT_SUCCESS(ntStatus))
	{
#ifdef _DEBUG
		TraceError("RtlAddAccessDeniedAce fail! Error: %p", ntStatus);
#endif
		return false;
	}

	if (SetSecurityInfo(GetCurrentProcess(), SE_KERNEL_OBJECT, PROTECTED_DACL_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION, 0, 0, pACL, 0) != ERROR_SUCCESS)
	{
#ifdef _DEBUG
		TraceError("SetSecurityInfo fail! Error: %u", GetLastError());
#endif
		return false;
	}
	return true;
}

bool CAnticheatManager::DenyThreadAccess()
{
	typedef NTSTATUS(NTAPI* TRtlAddAccessDeniedAce)(PACL pAcl, uint32_t dwAceRevision, ACCESS_MASK	AccessMask, PSID pSid);

	auto hNtdll = LoadLibraryA(XOR("ntdll.dll"));
	if (!hNtdll)
		return false;

	auto lpRtlAddAccessDeniedAce = GetProcAddress(hNtdll, XOR("RtlAddAccessDeniedAce"));
	if (!lpRtlAddAccessDeniedAce)
		return false;
	auto RtlAddAccessDeniedAce = reinterpret_cast<TRtlAddAccessDeniedAce>(lpRtlAddAccessDeniedAce);

	SID pSid = { 0 };
	pSid.Revision = SID_REVISION;
	pSid.IdentifierAuthority = SECURITY_WORLD_SID_AUTHORITY;
	pSid.SubAuthorityCount = 1;
	pSid.SubAuthority[0] = SECURITY_WORLD_RID;

	uint8_t pBuffer[0x400] = { 0 };

	auto pACL = (PACL)&pBuffer;
	pACL->AclRevision = ACL_REVISION;
	pACL->AclSize = sizeof(pBuffer);
	pACL->AceCount = 0;

	auto ntStatus = RtlAddAccessDeniedAce(pACL, ACL_REVISION, THREAD_POISONED_ACCESS, &pSid);
	if (!NT_SUCCESS(ntStatus))
	{
#ifdef _DEBUG
		TraceError("RtlAddAccessDeniedAce fail! Error: %p", ntStatus);
#endif
		return false;
	}

	if (SetSecurityInfo(GetCurrentThread(), SE_KERNEL_OBJECT, PROTECTED_DACL_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION, 0, 0, pACL, 0) != ERROR_SUCCESS)
	{
#ifdef _DEBUG
		TraceError("SetSecurityInfo fail! Error: %u", GetLastError());
#endif
		return false;
	}
	return true;
}

