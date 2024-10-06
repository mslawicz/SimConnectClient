#include "Console.h"
#include "HidDevice.h"
#include "Simulator.h"
#include <iostream>
#include <thread>
//#include <functional>

#define VENDOR_ID   0x483
#define PRODUCT_ID  22369
#define REPORT_ID   0x02

int main()
{
    Console::getInstance().log(LogLevel::Always, "SimConnect Client v2.0");
    Console::getInstance().log(LogLevel::Always, "type 'help' for the list of commands");

    HidDevice hidDevice(VENDOR_ID, PRODUCT_ID, REPORT_ID);
    Simulator::getInstance().setHidDevice(&hidDevice);
    //hidDevice.setParseFunction(std::bind(&Simulator::parseReceivedData, &Simulator::getInstance(), std::placeholders::_1));

    std::thread hidDeviceThread(&HidDevice::handler, &hidDevice);
    std::thread simulatorThread(&Simulator::handler, &Simulator::getInstance());

    Console::getInstance().handler();

    simulatorThread.join();
    hidDeviceThread.join();

    return 0;
}