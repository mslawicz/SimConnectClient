#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#include "SimConnect.h"
#include <iostream>

HANDLE hSimConnect = NULL;

int main(void)
{
    SimConnect_Open(&hSimConnect, "Throttle Control", NULL, 0, 0, 0);   //example of a command
    std::cout << "Hello World!" << std::endl;
    return 0;
}