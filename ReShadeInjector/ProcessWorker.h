#pragma once
#include "InjectionStatus.h"
#include <windows.h>
#include <TlHelp32.h>

namespace ProcessWorker
{
    extern auto GetProcessId(const wchar_t* szProcessName) -> int;
    extern auto InjectToProcess(const wchar_t* szProcessName, wchar_t* szLibraryName) -> InjectionStatus;
}
