#pragma comment(lib, "SetupAPI.lib")

#include "HidDevice.h"
#include "Console.h"
#include <SetupAPI.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

HidDevice::HidDevice(USHORT VID, USHORT PID, uint8_t collection) :
    VID(VID),
    PID(PID),
    collection(collection)
{
    memset(&receiveOverlappedData, 0, sizeof(receiveOverlappedData));
    receiveOverlappedData.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    memset(&sendOverlappedData, 0, sizeof(sendOverlappedData));
    sendOverlappedData.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    std::stringstream ss;
    ss << std::hex << "USB HID device with VID=" << VID << " PID=" << PID;
    VidPid = ss.str();
    collectionStr = "&col";
    if (collection < 10)
    {
        collectionStr += "0";
    }
    collectionStr += std::to_string(collection);
}

HidDevice::~HidDevice()
{
}

// USB link handler to be called in a separate thread
void HidDevice::handler()
{
    // stay in this loop until the user requests quit
    while (!Console::getInstance().isQuitRequest())
    {
        if (isOpen)
        {
            // check a new data from HID device
            if (isDataReceived())
            {
                // call reveived data parsing function
                if (parseCallback)
                {
                    parseCallback(std::vector<uint8_t>(receiveBuffer, receiveBuffer + HidBufferSize));
                }
                enableReception();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(ConnectionOnPeriod));
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(ConnectionOffPeriod));
            // no USB connection - try to connect
            if (openConnection())
            {
                // connection has been opened
                // enable reception for the first time
                if (enableReception())
                {
                    Console::getInstance().log(LogLevel::Info, "USB data reception enabled");
                }
                else
                {
                    Console::getInstance().log(LogLevel::Error, "USB data reception enabling failed");
                }
            }
        }
    }

    if (isOpen)
    {
        disableReception();
        closeConnection();
    }
}

// find the USB device and open the connection to it
bool HidDevice::openConnection()
{
    isOpen = false;
    bool found = false;     // mark that the device has not been found yet

    HidD_GetHidGuid(&hidGuid);      // get the device interfaceGUID for HIDClass devices

    // SetupDiGetClassDevs function returns a handle to a device information set that contains requested device information elements for a local computer
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
    if (deviceInfoSet == INVALID_HANDLE_VALUE)
    {
        Console::getInstance().log(LogLevel::Error, "Invalid handle to device information set, error code=" + std::to_string(GetLastError()));
        return isOpen;
    }

    SP_DEVINFO_DATA deviceInfoData;     //  structure defines a device instance that is a member of a device information set
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    // SetupDiEnumDeviceInfo function returns a SP_DEVINFO_DATA structure that specifies a device information element in a device information set
    for (int deviceIndex = 0; SetupDiEnumDeviceInfo(deviceInfoSet, deviceIndex, &deviceInfoData) && !found; deviceIndex++)
    {
        SP_DEVICE_INTERFACE_DATA devInterfaceData;  // structure defines a device interface in a device information set
        devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        // SetupDiEnumDeviceInterfaces function enumerates the device interfaces that are contained in a device information set
        for (int interfaceIndex = 0; SetupDiEnumDeviceInterfaces(deviceInfoSet, &deviceInfoData, &hidGuid, interfaceIndex, &devInterfaceData) && !found; interfaceIndex++)
        {
            DWORD bufferSize = 0;
            // SetupDiGetDeviceInterfaceDetail function returns details about a device interface
            // this first call is for getting required size of the DeviceInterfaceDetailData buffer
            SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &devInterfaceData, NULL, 0, &bufferSize, &deviceInfoData);

            // pointer to buffer that receives information about the device that supports the requested interface
            PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(bufferSize);
            if (pDeviceInterfaceDetailData != nullptr)
            {
                pDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            }
            else
            {
                Console::getInstance().log(LogLevel::Error, "Memory allocation error");
                continue;
            }

            // SetupDiGetDeviceInterfaceDetail function returns details about a device interface
            // this second call is for getting actual information about the device that supports the requested interface
            if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &devInterfaceData, pDeviceInterfaceDetailData, bufferSize, &bufferSize, &deviceInfoData))
            {
                SECURITY_ATTRIBUTES securityAttributes;
                memset(&securityAttributes, 0, sizeof(SECURITY_ATTRIBUTES));
                securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
                securityAttributes.bInheritHandle = true;

                std::string devicePath(pDeviceInterfaceDetailData->DevicePath);
                Console::getInstance().log(LogLevel::Debug, "checking " + devicePath);

                // Creates or opens a file or I/O device
                // query metadata such as file, directory, or device attributes without accessing device 
                fileHandle = CreateFile(pDeviceInterfaceDetailData->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    &securityAttributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                if (fileHandle != INVALID_HANDLE_VALUE)
                {
                    HIDD_ATTRIBUTES	attributes;
                    memset(&attributes, 0, sizeof(HIDD_ATTRIBUTES));
                    attributes.Size = sizeof(HIDD_ATTRIBUTES);
                    if (HidD_GetAttributes(fileHandle, &attributes))
                    {
                        CloseHandle(fileHandle);
                        if ((attributes.VendorID == VID) &&
                            (attributes.ProductID == PID) &&
                            ((collection == 0) || (devicePath.find(collectionStr) != std::string::npos)))
                        {
                            // device with proper VID, PID and collection found (or collection == 0)
                            // Creates or opens a file or I/O device - this time for read/write operations in asynchronous mode
                            fileHandle = CreateFile(pDeviceInterfaceDetailData->DevicePath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                &securityAttributes, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
                            if (fileHandle != INVALID_HANDLE_VALUE)
                            {
                                isOpen = true;
                                std::stringstream ss;
                                ss << "Connection to " << VidPid.c_str();
                                if (collection)
                                {
                                    ss << " collection=" << static_cast<int>(collection);
                                }
                                ss << " opened";
                                Console::getInstance().log(LogLevel::Info, ss.str());
                            }
                            else
                            {
                                Console::getInstance().log(LogLevel::Error, "USB device found, but cannot be opened for read/write operations, error code=" + std::to_string(GetLastError()));
                            }
                            // mark that the device has been found regardless if it has been opened
                            found = true;
                        }
                    }
                }
                else
                {
                    std::stringstream ss;
                    ss << "invalid file handle, error code = " << GetLastError();
                    Console::getInstance().log(LogLevel::Warning, ss.str());
                }
            }
            else
            {
                std::stringstream ss;
                ss << "couldn't get device interface details; error code = " << GetLastError();
                Console::getInstance().log(LogLevel::Warning, ss.str());
            }
            free(pDeviceInterfaceDetailData);
        }
    }
    SetupDiDestroyDeviceInfoList(deviceInfoSet);

    if (!isOpen)
    {
        Console::getInstance().log(LogLevel::Debug, VidPid + " not found");
    }

    return isOpen;
}


// closes connection to the device
void HidDevice::closeConnection()
{
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(fileHandle);
        fileHandle = INVALID_HANDLE_VALUE;
        Console::getInstance().log(LogLevel::Info, "closing connection to " + VidPid);
    }
    else
    {
        Console::getInstance().log(LogLevel::Debug, "Invalid handle to HID device");
    }
    isOpen = false;
}


// starts reception in asynchronous mode
// this way it enables reception of the incoming data
bool HidDevice::enableReception(void)
{
    if (isOpen && (fileHandle != INVALID_HANDLE_VALUE))
    {
        auto result = ReadFile(fileHandle, receiveBuffer, HidBufferSize, &receivedDataCount, &receiveOverlappedData);
        DWORD lastError = GetLastError();
        if ((result != 0) && (lastError != 997))
        {
            // when OK, expected values are result=0, receivedDataCount=HidBufferSize or 0 and err=997
            std::stringstream ss;
            ss << "USB read result=" << result << " cnt=" << receivedDataCount << " error=" << GetLastError();
            Console::getInstance().log(LogLevel::Warning, ss.str());
            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}

// disable USB reception
void HidDevice::disableReception(void)
{
    auto result = ResetEvent(receiveOverlappedData.hEvent);  // clears the reception event (no signals until enabled again)
    Console::getInstance().log(LogLevel::Info, "USB reading disabled with code " + std::to_string(result));
}

// return true if received data is signaled
// this call doesn't reset the signal
bool HidDevice::isDataReceived(void)
{
    return (WaitForSingleObject(receiveOverlappedData.hEvent, 0) == WAIT_OBJECT_0);
}

// send data to USB HID device
bool HidDevice::sendData(uint8_t* dataToSend)
{
    if (isOpen && (fileHandle != INVALID_HANDLE_VALUE))
    {
        // get overlapped result without waiting
        bool overlappedResult = GetOverlappedResult(fileHandle, &sendOverlappedData, &sendDataCount, FALSE);
        DWORD lastError = GetLastError();
        // if the process is pending, return without action
        if (!overlappedResult && lastError == ERROR_IO_PENDING)
        {
            Console::getInstance().log(LogLevel::Error, "USB data send error=" + std::to_string(lastError) + " (pending)", ONLY_ONCE);
            return false;
        }
        // if the process is not over, but it's not pending (the other error occured)
        else if (!overlappedResult)
        {
            // reset overlapped data
            memset(&sendOverlappedData, 0, sizeof(sendOverlappedData));
            sendOverlappedData.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            Console::getInstance().log(LogLevel::Error, "USB data send error=" + std::to_string(lastError), ONLY_ONCE);
            if (++sendErrorCounter >= SendErrorLimit)
            {
                closeConnection();
                sendErrorCounter = 0;
                return false;
            }
        }
        else
        {
            // overlapped result true
            sendErrorCounter = 0;
        }
        // send data every time if only process in not pending
        sendBuffer[0] = collection;
        memcpy(sendBuffer+1, dataToSend, HidBufferSize-1);
        WriteFile(fileHandle, sendBuffer, HidBufferSize, NULL, &sendOverlappedData);
        return true;
    }
    return false;
}