#pragma once
#include "InjectionStatus.h"

namespace ProcessWorker
{
	extern int GetProcessId(const wchar_t* pName);
	extern InjectionStatus InjectToProcess(const wchar_t* pName, const wchar_t* libName);
}