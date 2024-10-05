#include "SimConnectHandler.h"
#include "SimConnectClient.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include "SimConnect.h"
#include <thread>
#include <chrono>
#include <iostream>

HANDLE hSimConnect = NULL;

void SimConnectHandler(void)
{
    while(!SimConnectClientQuit)
    {
        std::cout << "handler loop" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return;
}