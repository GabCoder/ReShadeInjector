#define NOMINMAX
#include "ProcessWorker.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream> // std::cin, std::cout
#include <limits> // std::numeric_limits<>
#include <cwchar> // std::wcsrchr

/**
 * \param szProcessName Name of the process. 
 * 
 * \return Process ID.
 */
auto ProcessWorker::GetProcessId(const wchar_t* szProcessName) -> int
{
    auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if( hSnapshot == INVALID_HANDLE_VALUE )
        return 0;

    PROCESSENTRY32 pe{sizeof(PROCESSENTRY32)};

    for(auto success = Process32First(hSnapshot, &pe);
        success == TRUE;
        success = Process32Next(hSnapshot, &pe)) 
    {
        if (wcscmp(pe.szExeFile, szProcessName) == 0) 
        {
            CloseHandle(hSnapshot);
            return static_cast<int>(pe.th32ProcessID);
        }
    }

    CloseHandle(hSnapshot);
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

	auto kernel32 = GetModuleHandle(L"kernel32.dll");
    if (!kernel32)
    {
        CloseHandle(hProcess);
        return FAILED_NOKERNEL32;
    }

    // Get LoadLibraryW function address.
    auto pLoadLibrary = GetProcAddress(kernel32, "LoadLibraryW");
    if (!pLoadLibrary)
    {
        CloseHandle(hProcess);
        return FAILED_NOLOADLIBRARY;
    }
	
	auto size = wcslen(szLibraryName) * 2;

    // Allocate memory for string.
	auto RemoteString = VirtualAllocEx(hProcess, nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!RemoteString)
    {
        CloseHandle(hProcess);
        return FAILED_VIRTUALALLOCEX;
    }

	if (WriteProcessMemory(hProcess, RemoteString, szLibraryName, size, nullptr) == FALSE) 
    {
        CloseHandle(hProcess);
        VirtualFreeEx(hProcess, RemoteString, 0, MEM_RELEASE);
        return FAILED_WRITEPROCESSMEMORY;
	}
		

    // Create thread in process that calls LoadLibraryW with our string.
    auto hThread = CreateRemoteThread(hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pLoadLibrary), RemoteString, 0, nullptr);
	if (!hThread) 
    {
        CloseHandle(hProcess);
        VirtualFreeEx(hProcess, RemoteString, 0, MEM_RELEASE);
        return FAILED_CREATEREMOTETHREAD;
	}
		

    // Wait until thread finish.
    if(WaitForSingleObjectEx(hThread, INFINITE, FALSE) == WAIT_FAILED) 
    {
        CloseHandle(hProcess);
        VirtualFreeEx(hProcess, RemoteString, 0, MEM_RELEASE);
        CloseHandle(hThread);
        return FAILED_CREATEREMOTETHREAD;
    }
    
    // Close thread handle.
    CloseHandle(hThread);

    // Free memory that we used.
	if (VirtualFreeEx(hProcess, RemoteString, 0, MEM_RELEASE) == FALSE)
	{
		CloseHandle(hProcess);
		return FAILED_VIRTUALFREE;
	}

    // Close process handle.
    CloseHandle(hProcess);

    // We succeed.
    return SUCCESS;
}

/**
* \param pBuf Buffer for the directory to be copied into
* \param nSize Size of the buffer
*
* \return The number of characters copied into the buffer. 0 on failure
*/
auto Utils::GetCurrentWorkingDirectory( wchar_t* pBuf, std::size_t nSize ) -> int 
{
    int copied = GetModuleFileNameW(nullptr, pBuf, static_cast<DWORD>(nSize));
    if (copied == 0)
        return 0;

    auto ptr = std::wcsrchr(pBuf, '\\') + 1;
    if (ptr < pBuf + nSize)
        *ptr = '\0';

    return ptr - pBuf;
}

/**
* \brief Waits for user to press any key
*
* \param message The message to be displayed
*/
auto Utils::WaitForInput( const char* message ) -> void 
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max());
    std::cout << message;
    std::cin.get();
}
