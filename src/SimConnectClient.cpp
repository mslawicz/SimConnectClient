//#include <windows.h>
//#include <tchar.h>
//#include <stdio.h>
//#include <strsafe.h>

//#include "SimConnect.h"
#include "SimConnectClient.h"
#include <iostream>
#include <conio.h>

int main(void)
{
    int userInput;
    std::cout << "SimConnect Client version 0.2.0" << std::endl;

    do
    {
        userInput = _getch();
        std::cout << static_cast<char>(userInput) << "_,_" << userInput << std::endl;
    } while (userInput != KbCodes::Kb_Ctrl_C);
    
    return 0;
}