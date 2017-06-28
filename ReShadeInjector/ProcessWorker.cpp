#include "ProcessWorker.h"
#include <Windows.h>
#include <TlHelp32.h>

int ProcessWorker::GetProcessId(const wchar_t* szName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 pe{ sizeof(PROCESSENTRY32) };
	Process32First(hSnapshot, &pe);

	if (wcscmp(pe.szExeFile, szName) == 0) 
		return static_cast<int>(pe.th32ProcessID);

	while (Process32Next(hSnapshot, &pe))
	{
		if (wcscmp(pe.szExeFile, szName) == 0)
			return static_cast<int>(pe.th32ProcessID);
	}
	return 0;
}

InjectionStatus ProcessWorker::InjectToProcess(const wchar_t * szName, const wchar_t * szLibName)
{
	auto pId = GetProcessId(szName);

	if (!pId)
		return FAILED_PROCESSNOTFOUND;

	HANDLE Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);
	if (!Proc)
		return FAILED_OPENPROCESS;

	auto LoadLibAddy = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

	auto RemoteString = VirtualAllocEx(Proc, nullptr, sizeof(szLibName), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(Proc, RemoteString, szLibName, sizeof(szLibName), nullptr);
	CreateRemoteThread(Proc, nullptr, NULL, reinterpret_cast< LPTHREAD_START_ROUTINE >(LoadLibAddy), RemoteString, NULL, nullptr);
	VirtualFreeEx(Proc, RemoteString, NULL, MEM_RELEASE);
	CloseHandle(Proc);
	return SUCCESS;
}

