#include "SimConnectClient.h"
#include "SimConnectHandler.h"
#include <iostream>
#include <conio.h>
#include <thread>

volatile bool SimConnectClientQuit = false;

int main(void)
{
    int userInput;
    std::thread SimConnectHandlerThread(SimConnectHandler);
    std::cout << "SimConnect Client version 0.2.0" << std::endl;

    do
    {
        userInput = _getch();
        std::cout << static_cast<char>(userInput) << "_,_" << userInput << std::endl;
    } while (userInput != KbCodes::Kb_Ctrl_C);

    SimConnectClientQuit = true;
    SimConnectHandlerThread.join();
    
    return 0;
}