#include <string.h>
#include <stdio.h>
#include <string>

#ifdef __GNUC__
typedef bool BOOL;
typedef unsigned int DWORD;

#define printf_s printf
#define _stricmp strcmp
#define _snprintf_s snprintf
#define _popen popen
#define fopen_s fopen
#define fread_s fread
//#define atoi(s) strtoul(s, NULL, 10)

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include <Windows.h>
#endif

std::string g_stPidPath = "pid";
std::string g_stFileName;

unsigned int g_dwRunningPID = 0;

bool interpret_argument(char* arg)
{
	char* szPos = strstr(arg, "=");
	int iArgNameLen = szPos - arg;

	if (!szPos || !iArgNameLen || *(szPos + 1) == '\0')
	{
		printf_s("ERROR: interpreting argument : %s\n", arg);
		return false;
	}

	char* szArgName = arg;
	char* szArgData = szPos + 1;
	*szPos = '\0';

	if (!_stricmp(szArgName, "--pid-path"))
		g_stPidPath = szArgData;
	else if (!_stricmp(szArgName, "--file"))
		g_stFileName = szArgData;
	else
		printf_s("Skip argument : %s=%s\n", szArgName, szArgData);

	return true;
}

BOOL IsProcessRunning(DWORD pid)
{
#ifdef __GNUC__
	char szPidFileName[FILENAME_MAX];
	_snprintf_s(szPidFileName, sizeof(szPidFileName), "/proc/%u", pid);

	struct stat sts;
	if (stat(szPidFileName, &sts) == -1)
		return false;

	return true;
#else
	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
	DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	return ret == WAIT_TIMEOUT;
#endif
}

bool BootApplication()
{
#ifdef __GNUC__
	FILE* pf;
	if (!(pf = popen(g_stFileName.c_str(), "r")))
		return false;

	FILE* pfPid;
	if (!(pfPid = fopen("pid", "r")))
	{
		printf_s("ERROR: cannot read pid file \"pid\"\n");
		return false;
	}

	char szDstBuf[4 + 1];
	szDstBuf[4] = '\0';
	fread(szDstBuf, sizeof(szDstBuf), 4, pfPid);
	g_dwRunningPID = atoi(szDstBuf);
	fclose(pfPid);

	pfPid = fopen(g_stPidPath.c_str(), "w");
	fwrite(szDstBuf, strlen(szDstBuf), 1, pfPid);
	fclose(pfPid);
#else
	std::wstring wstTempFileName = std::wstring(g_stFileName.begin(), g_stFileName.end());

	PROCESS_INFORMATION pi { 0 };
	STARTUPINFOW si { 0 };
	si.cb = sizeof(si);

	CreateProcessW(wstTempFileName.c_str(),   // the path
		L"\0",          // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi);           // Pointer to PROCESS_INFORMATION structure

	g_dwRunningPID = pi.dwProcessId;
#endif

	return g_dwRunningPID != 0;
}

void OnAfterBootApplication()
{
	// write pid file
	FILE* fp;
#ifdef __GNUC__
	if (!(fp = fopen(g_stPidPath.c_str(), "w")))
#else
	if (fopen_s(&fp, g_stPidPath.c_str(), "w"))
#endif
	{
		printf_s("ERROR: cannot write pid file %s\n", g_stPidPath.c_str());
		return;
	}

	char szBuf[20];
	_snprintf_s(szBuf, sizeof(szBuf), "%u", g_dwRunningPID);
	fwrite(szBuf, strlen(szBuf), 1, fp);

	fclose(fp);
}

int main(int argc, char* argv[])
{
	printf_s("Boot TFP-VRUNNER...\n");

	for (int i = 1; i < argc; ++i)
	{
		if (!interpret_argument(argv[i]))
			return 0;
	}

	if (g_stFileName.empty())
	{
		printf_s("ERROR: no run file name given (use --file)\n");
		return 0;
	}

	if (!BootApplication())
	{
		printf_s("ERROR: cannot run file %s\n", g_stFileName.c_str());
		return 0;
	}
	OnAfterBootApplication();

	while (1)
	{
		if (!IsProcessRunning(g_dwRunningPID))
		{
			printf_s("*** REBOOT application %s\n", g_stFileName.c_str());
			if (!BootApplication())
			{
				printf_s("ERROR: cannot restart file %s\n", g_stFileName.c_str());
				return 0;
			}
			OnAfterBootApplication();
		}

#ifdef __GNUC__
		sleep(5000);
#else
		Sleep(5000);
#endif
	}

	printf_s("Shutdown TFP-VRUNNER...\n");

	return 0;
}

