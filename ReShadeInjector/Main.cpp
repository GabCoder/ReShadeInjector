#include <iostream>
#include "ProcessWorker.h"

void main()
{
	system("color F3");
	std::cout << "Reshade injector by ChangerTeam | reshade.me\n";

	if (ProcessWorker::GetProcessId(L"csgo.exe") != 0)
	{
		system("color FC");
		std::cout << "csgo.exe is already started! => reshade injection failed :(\n";
		_sleep(2000);
		return;
	}

	std::cout << "Looking for csgo.exe process...\n";

	do
	{
		_sleep(13);
	} while (!ProcessWorker::GetProcessId(L"csgo.exe"));

	std::cout << "Injection status -> ";
	
	InjectionStatus feedback = ProcessWorker::InjectToProcess(L"csgo.exe", "d3d9.dll");

	switch (feedback)
	{
	case SUCCESS:
		system("color F2");
		std::cout << "success!";
		break;
	case FAILED_OPENPROCESS:
		system("color FC");
		std::cout << "failed (OpenProcess error)!";
		break;
	case FAILED_PROCESSNOTFOUND:
		system("color FC");
		std::cout << "failed (process not found)!";
		break;
	}
	_sleep(2017);
}
