#include "ProcessWorker.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <cassert>

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
	assert(pName);
	assert(libName);

	int pId = GetProcessId(pName);

	if (!pId)
		return FAILED_PROCESSNOTFOUND;
	
	size_t libName_len = strlen(libName);

	HMODULE kernel32_handle = GetModuleHandleW(L"kernel32.dll");
	if (!kernel32_handle)
	{
		return FAILED_NOKERNEL32;
	}

	LPVOID LoadLibAddy = static_cast<LPVOID>(GetProcAddress(kernel32_handle, "LoadLibraryA"));
	if (!LoadLibAddy)
	{
		return FAILED_NOLOADLIBRARY;
	}

	HANDLE Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);
	if (!Proc)
	{
		return FAILED_OPENPROCESS;
	}

	LPVOID RemoteString = static_cast<LPVOID>(VirtualAllocEx(Proc, nullptr, libName_len, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
	if(!RemoteString)
	{
		CloseHandle(Proc);
		return FAILED_VIRTUALALLOCEX;
	}

	if(WriteProcessMemory(Proc, static_cast<LPVOID>(RemoteString), libName, libName_len, nullptr) == FALSE)
	{
		VirtualFreeEx(Proc, RemoteString, libName_len, MEM_RESERVE | MEM_COMMIT);
		CloseHandle(Proc);
		return FAILED_WRITEPROCESSMEMORY;
	}

	HANDLE hRemoteThread = CreateRemoteThread(Proc, nullptr, NULL, static_cast<LPTHREAD_START_ROUTINE>(LoadLibAddy), static_cast<LPVOID>(RemoteString), NULL, nullptr);
	if(!hRemoteThread)
	{
		VirtualFreeEx(Proc, RemoteString, libName_len, MEM_RESERVE | MEM_COMMIT);
		CloseHandle(Proc);
		return FAILED_CREATEREMOTETHREAD;
	}

	WaitForSingleObject(hRemoteThread, INFINITE);
	CloseHandle(hRemoteThread);
	VirtualFreeEx(Proc, RemoteString, libName_len, MEM_RESERVE | MEM_COMMIT);
	CloseHandle(Proc);
	return SUCCESS;
}

