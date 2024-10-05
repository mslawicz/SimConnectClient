#include "SimConnectHandler.h"
#include "SimConnectClient.h"
#include <thread>
#include <chrono>
#include <iostream>

void SimConnectHandler(void)
{
    while(!SimConnectClientQuit)
    {
        std::cout << "handler loop" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return;
}