#pragma once
#include <cstddef> // std::size_t
#include "InjectionStatus.h"

namespace ProcessWorker
{
    /**
    * \param szProcessName Name of the process.
    *
    * \return Process ID.
    */
    auto GetProcessId(const wchar_t* szProcessName) -> int;

    /**
    * \param szProcessName Name of the process.
    * \param szLibraryName Name of the library to be injected.
    *
    * \return Injection status.
    */
    auto InjectToProcess(const wchar_t* szProcessName, const wchar_t* szLibraryName) -> InjectionStatus;
}

namespace Utils
{
    /**
     * \param pBuf Buffer for the directory to be copied into
     * \param nSize Size of the buffer
     * 
     * \return The number of characters copied into the buffer. 0 on failure
     */
    auto GetCurrentWorkingDirectory(wchar_t* pBuf, std::size_t nSize) -> int;

    /**
     * \brief Waits for user to press any key
     * 
     * \param message The message to be displayed
     */
    auto WaitForInput(const char* message) -> void;
}
