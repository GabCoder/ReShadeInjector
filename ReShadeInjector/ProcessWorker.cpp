#include "ProcessWorker.h"
#include <Windows.h>
#include <TlHelp32.h>

int ProcessWorker::GetProcessId(const wchar_t* pName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 pe{ sizeof(PROCESSENTRY32) };
	Process32First(hSnapshot, &pe);

	if (wcscmp(pe.szExeFile, pName) == 0) 
		return pe.th32ProcessID;

	while (Process32Next(hSnapshot, &pe))
	{
		if (wcscmp(pe.szExeFile, pName) == 0)
			return pe.th32ProcessID;
	}
	return 0;
}

InjectionStatus ProcessWorker::InjectToProcess(const wchar_t * pName, const char * libName)
{
	int pId = GetProcessId(pName);

	if (!pId)
		return FAILED_PROCESSNOTFOUND;

	HANDLE Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);
	if (!Proc)
		return FAILED_OPENPROCESS;

	LPVOID LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

	LPVOID RemoteString = (LPVOID)VirtualAllocEx(Proc, nullptr, strlen(libName), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(Proc, (LPVOID)RemoteString, libName, strlen(libName), nullptr);
	CreateRemoteThread(Proc, nullptr, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, nullptr);

	CloseHandle(Proc);
	return SUCCESS;
}

