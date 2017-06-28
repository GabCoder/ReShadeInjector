#pragma once
#include "InjectionStatus.h"

namespace ProcessWorker
{
    extern auto GetProcessId(const wchar_t* szProcessName) -> int;
    extern auto InjectToProcess(const wchar_t* szProcessName, const wchar_t* szLibraryName) -> InjectionStatus;
}
