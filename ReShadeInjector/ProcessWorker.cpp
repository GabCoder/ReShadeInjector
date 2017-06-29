#include "ProcessWorker.h"

/**
 * \param szProcessName Name of the process. 
 * 
 * \return Process ID.
 */
auto ProcessWorker::GetProcessId(const wchar_t* szProcessName) -> int
{
    auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if( !hSnapshot )
        return 0;

    PROCESSENTRY32 pe{sizeof(PROCESSENTRY32)};
    Process32First(hSnapshot, &pe);

    if (wcscmp(pe.szExeFile, szProcessName) == 0)
        return static_cast<int>(pe.th32ProcessID);

    while (Process32Next(hSnapshot, &pe))
    {
        if (wcscmp(pe.szExeFile, szProcessName) == 0)
            return static_cast<int>(pe.th32ProcessID);
    }

    return 0;
}

/**
 * \param szProcessName Name of the process.
 * \param szLibraryName Name of the library to be injected.
 * 
 * \return Injection status.
 */
auto ProcessWorker::InjectToProcess(const wchar_t* szProcessName, wchar_t* szLibraryName) -> InjectionStatus
{
    // Get ID of process.
    auto iPID = GetProcessId(szProcessName);
    if (!iPID)
        return FAILED_PROCESSNOTFOUND;

    // Get process handle using PID.
    auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iPID);
	if (!hProcess)
		return FAILED_OPENPROCESS;

	HMODULE kernel32 = GetModuleHandle(L"kernel32.dll");
	if (!kernel32)
		return FAILED_NOKERNEL32;

    // Get LoadLibraryW function address.
    auto pLoadLibrary = GetProcAddress(kernel32, "LoadLibraryW");
    if (!pLoadLibrary)
        return FAILED_NOLOADLIBRARY;
	
	size_t size = wcslen(szLibraryName) * 2;

    // Allocate memory for string.
	LPVOID RemoteString = VirtualAllocEx(hProcess, nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!RemoteString)
		return FAILED_VIRTUALALLOCEX;

	if (WriteProcessMemory(hProcess, RemoteString, szLibraryName, size, nullptr) == FALSE)
		return FAILED_WRITEPROCESSMEMORY;

    // Create thread in process that calls LoadLibraryW with our string.
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(pLoadLibrary), RemoteString, NULL, nullptr);
	if (!hThread)
		return FAILED_CREATEREMOTETHREAD;

    // Wait until thread finish.
    WaitForSingleObjectEx(hThread, INFINITE, FALSE);
    
    // Close thread handle.
    CloseHandle(hThread);

    // Free memory that we used.
	if (VirtualFreeEx(hProcess, RemoteString, NULL, MEM_RELEASE) == FALSE)
	{
		CloseHandle(hProcess);
		return FAILED_VIRTUALFREE;
	}

    // Close process handle.
    CloseHandle(hProcess);

    // We succeed.
    return SUCCESS;
}
