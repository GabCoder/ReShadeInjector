#include <iostream>
#include "ProcessWorker.h"

int main()
{
	system("color F3");
	std::cout << "Reshade injector by ChangerTeam | reshade.me\n";

	if (ProcessWorker::GetProcessId(L"csgo.exe") != 0)
	{
		system("color FC");
		std::cout << "csgo.exe is already started! => reshade injection failed :(\n";
		_sleep(2000);
		return EXIT_FAILURE; // у меня таке было, библиотека какая-то коряво подклбчена!
	}

	std::cout << "Looking for csgo.exe process...\n";

	do
	{
		_sleep(13);
	}
	while (!ProcessWorker::GetProcessId(L"csgo.exe"));

	std::cout << "Injection status -> ";

	wchar_t working_dir[MAX_PATH];
	DWORD rl = GetModuleFileNameW(GetModuleHandleW(nullptr), working_dir, MAX_PATH);
	DWORD i;
	for (i = rl - 1; i != 0; i--)
	{
		if (working_dir[i] == L'\\')
			break;
		working_dir[i] = L'\0';
	}
	wcscpy(&working_dir[i+1], L"d3d9.dll");

	auto eExitStatus = ProcessWorker::InjectToProcess(L"csgo.exe", working_dir);
	switch (eExitStatus)
	{
	case SUCCESS:
		system("color F2");
		std::cout << "success!";
		break;
	case FAILED_OPENPROCESS:
		system("color FC");
		std::cout << "failed! (OpenProcess error)";
		break;
	case FAILED_PROCESSNOTFOUND:
		system("color FC");
		std::cout << "failed! (process not found)";
		break;
	case FAILED_NOLOADLIBRARY:
		system("color FC");
		std::cout << "failed! (LoadLibrary not found)";
		break;
	case FAILED_NOKERNEL32:
		system("color FC");
		std::cout << "failed! (kernel32.dll not found)";
		break;
	case FAILED_WRITEPROCESSMEMORY:
		system("color FC");
		std::cout << "failed! (not able to write a process memory)";
		break;
	case FAILED_CREATEREMOTETHREAD:
		system("color FC");
		std::cout << "failed! (not able to create remote thread)";
		break;
	case FAILED_VIRTUALALLOCEX:
		system("color FC");
		std::cout << "failed! (not able to alloc space at csgo.exe)";
		break;
	}
	_sleep(2017);
	return EXIT_SUCCESS;
}
