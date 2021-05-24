#include <Windows.h>
#include <DbgHelp.h>
#include <CrashRpt.h>
#include <tchar.h>

extern LONG WINAPI sehFilter(PEXCEPTION_POINTERS ExceptionInfo);

int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo)
{
	if (pInfo && pInfo->pExceptionInfo && pInfo->pExceptionInfo->pexcptrs)
		sehFilter(pInfo->pExceptionInfo->pexcptrs);

	return CR_CB_DODEFAULT;
}

// Install crash reporting
bool SetEterExceptionHandler()
{
	CR_INSTALL_INFO info{};
	info.cb = sizeof(CR_INSTALL_INFO);

	info.pszAppName = _T("Titan2"); // Define application name.
	info.pszAppVersion = _T("1.0.0"); // Define application version.
	info.pszEmailSubject = _T("Client Error Report");
	info.pszEmailTo = _T("mail@mail.mail");
	//	info.pszUrl = _T("http://127.0.0.1/crashrpt.php");

		// Install all available exception handlers
	info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;

	info.dwFlags |= CR_INST_APP_RESTART; // Restarts the app after crash
	info.dwFlags |= CR_INST_SEND_QUEUED_REPORTS;
	info.pszRestartCmdLine = "/restart";

	// Provide privacy policy URL
	info.pszPrivacyPolicyURL = _T("http://127.0.0.1/err_privacy.html");

	info.uPriorities[CR_HTTP] = 3; // First try send report over HTTP
	info.uPriorities[CR_SMTP] = 2; // Second try send report over SMTP
	info.uPriorities[CR_SMAPI] = 1; // Third try send report over Simple MAPI

	auto nResult = crInstall(&info);
	if (nResult != 0)
	{
		TCHAR buff[256];
		crGetLastErrorMsg(buff, 256);

		MessageBox(nullptr, buff, _T("crInstall error"), MB_OK);
		return false;
	}

	// Set crash callback function
	crSetCrashCallback(CrashCallback, nullptr);

	// Take screenshot of the app window at the moment of crash
	crAddScreenshot2(CR_AS_VIRTUAL_SCREEN | CR_AS_USE_JPEG_FORMAT, CR_AV_QUALITY_LOW);
#ifdef _DEBUG
	crAddFile2(_T("log.txt"), nullptr, _T("Log File"), CR_AF_MAKE_FILE_COPY);
#endif
	crAddFile2(_T("syserr.txt"), nullptr, _T("ErrorLog File"), CR_AF_MAKE_FILE_COPY);
	return true;
}

bool RemoveEterException()
{
	crUninstall();
	return true;
}
