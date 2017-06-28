#include "ProcessWorker.h"
#include <Windows.h>
#include <TlHelp32.h>

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
auto ProcessWorker::InjectToProcess(const wchar_t* szProcessName, const wchar_t* szLibraryName) -> InjectionStatus
{
    // Get ID of process.
    auto iPID = GetProcessId(szProcessName);
    if (!iPID)
        return FAILED_PROCESSNOTFOUND;

    // Get process handle using PID.
    auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iPID);
    if (!hProcess)
        return FAILED_OPENPROCESS;

    // Get LoadLibraryW function address.
    auto pLoadLibrary = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
    if (!pLoadLibrary)
        return FAILED_LOADLIBRARYFOUND;

    // Allocate memory for string.
    auto RemoteString = VirtualAllocEx(hProcess, nullptr, sizeof szLibraryName, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    // Write string to memory.
    WriteProcessMemory(hProcess, RemoteString, szLibraryName, sizeof szLibraryName, nullptr);

    // Creathe thread in process that calls LoadLibraryW with out string.
    auto hThread = CreateRemoteThread(hProcess, nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(pLoadLibrary), RemoteString, NULL, nullptr);

    // Wait until thread finish.
    WaitForSingleObjectEx(hThread, INFINITE, FALSE);
    
    // Close thread handle.
    CloseHandle(hThread);

    // Free memory that we used.
    VirtualFreeEx(hProcess, RemoteString, NULL, MEM_RELEASE);

    // Close process handle.
    CloseHandle(hProcess);

    // We succeed.
    return SUCCESS;
}
