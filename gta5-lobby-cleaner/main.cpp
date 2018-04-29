#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>

DWORD GetFirstProcessID(TCHAR* cName);
BOOL SwitchProcessState(DWORD dwOwnerPID, BOOL bState);
void printError(TCHAR* msg);

int main(void)
{
	DWORD dwProcessID = GetFirstProcessID(TEXT("GTA5.exe"));
	if (dwProcessID != -1) {
		_tprintf(TEXT("Please wait for 10 seconds...\n"));
		Sleep(1000);
		SwitchProcessState(dwProcessID, false);
		for (unsigned int i = 1; i < 10; i++)
		{
			_tprintf(TEXT("%i "), i);
			Sleep(1000);
		}
		_tprintf(TEXT("\n"));
		SwitchProcessState(dwProcessID, true);
		_tprintf(TEXT("Process has been successfully resumed\n"));
	}

	system("pause");
	return 0;
}

DWORD GetFirstProcessID(TCHAR* cName)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
		return -1;
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		printError(TEXT("Process32First")); // show cause of failure
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return -1;
	}

	// Now walk the snapshot of processes
	do
	{
		if (!_tcscmp(pe32.szExeFile, cName))
		{
			_tprintf(TEXT("%i %s (%d threads)\n")
				, pe32.th32ProcessID, pe32.szExeFile, pe32.cntThreads);
			return pe32.th32ProcessID;
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return -1;
}

BOOL SwitchProcessState(DWORD dwOwnerPID, BOOL bState)
{
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	THREADENTRY32 te32;
	HANDLE hThread;

	// Take a snapshot of all running threads  
	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return(FALSE);

	// Fill in the size of the structure before using it. 
	te32.dwSize = sizeof(THREADENTRY32);

	// Retrieve information about the first thread,
	// and exit if unsuccessful
	if (!Thread32First(hThreadSnap, &te32))
	{
		printError(TEXT("Thread32First")); // show cause of failure
		CloseHandle(hThreadSnap);          // clean the snapshot object
		return(FALSE);
	}

	// Now walk the thread list of the system
	do
	{
		if (te32.th32OwnerProcessID == dwOwnerPID)
		{
			hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
			if (bState)
			{
				//_tprintf(TEXT("Resuming 0x%08X thread\n"), te32.th32ThreadID);
				ResumeThread(hThread);
			}
			else
			{
				//_tprintf(TEXT("Suspending 0x%08X thread\n"), te32.th32ThreadID);
				SuspendThread(hThread);
			}
			CloseHandle(hThread);
		}
	} while (Thread32Next(hThreadSnap, &te32));

	CloseHandle(hThreadSnap);
	return(TRUE);
}

void printError(TCHAR* msg)
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		sysMsg, 256, NULL);

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) &&
		((*p == '.') || (*p < 33)));

	// Display the message
	_tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}
