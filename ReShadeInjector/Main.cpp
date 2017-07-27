#include <iostream> // std::cin, std::cout
#include <cstdlib> // std::system, std::wcscat
#include <chrono> // std::chrono::milliseconds
#include <thread> // std::this_thread
#include "ProcessWorker.h"

int main()
{
	std::system("color F3");
	std::cout << "Reshade injector by ChangerTeam | reshade.me\n";

	if (ProcessWorker::GetProcessId(L"csgo.exe") != 0)
	{
		system("color FC");
        Utils::WaitForInput("csgo.exe is already started! => reshade injection failed :(\n");
		return EXIT_FAILURE; // у меня таке было, библиотека какая-то коряво подклбчена!
	}

	std::cout << "Looking for csgo.exe process...\n";

	do
	{
        std::this_thread::sleep_for(std::chrono::milliseconds(13));
	}
	while (!ProcessWorker::GetProcessId(L"csgo.exe"));

	std::cout << "Injection status -> ";

	wchar_t working_dir[256];
    auto working_dir_len = Utils::GetCurrentWorkingDirectory(working_dir, 256);
    if(working_dir_len == 0) 
    {
        Utils::WaitForInput("Failed to get current working directory!");
        return EXIT_FAILURE;
    }
    std::wcscat(working_dir, L"d3d9.dll");
    
	auto eExitStatus = ProcessWorker::InjectToProcess(L"csgo.exe", working_dir);
    if(eExitStatus == SUCCESS) 
    {
        std::system("color F2");
        Utils::WaitForInput("success!");
        return EXIT_SUCCESS;
    }

    std::system("color FC");
    switch(eExitStatus) 
    {
        case FAILED_PROCESSNOTFOUND:
            Utils::WaitForInput("failed! (process not found)");
            break;
        case FAILED_NOKERNEL32: 
            Utils::WaitForInput("failed! (kernel32.dll not found)");
            break;
        case FAILED_NOLOADLIBRARY:
            Utils::WaitForInput("failed! (LoadLibrary not found)");
            break;
        case FAILED_OPENPROCESS:
            Utils::WaitForInput("failed! (OpenProcess error)");
            break;
        case FAILED_VIRTUALALLOCEX:
            Utils::WaitForInput("failed! (unable to allocate space at csgo.exe)");
            break;
        case FAILED_WRITEPROCESSMEMORY:
            Utils::WaitForInput("failed! (unable to write a process memory)");
            break;
        case FAILED_CREATEREMOTETHREAD:
            Utils::WaitForInput("failed! (uanable to create remote thread)");
            break;
        case FAILED_WAITFORSINGLEOBJECTEX:
            Utils::WaitForInput("failed! (uable to wait for remote thread)");
            break;
        case FAILED_VIRTUALFREE:
            Utils::WaitForInput("failed! (uable to free allocated space at csgo.exe)");
            break;
        default:
            std::abort();
    }

	return EXIT_FAILURE;
}
