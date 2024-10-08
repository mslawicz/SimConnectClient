#pragma once
#pragma comment(lib, "hid.lib")

#include <Windows.h>
#include <cstdint>
#include <hidsdi.h>
#include <string>
#include<vector>
#include <functional>


class HidDevice
{
public:
    HidDevice(USHORT VID, USHORT PID, uint8_t collection);
    ~HidDevice();
    void handler();
    bool openConnection();
    void closeConnection();
    bool isConnectionOpen() const { return isOpen; }
    bool enableReception(void);
    void disableReception(void); // clears the reception event (no signals until enabled again)
    bool isDataReceived(void);
    void setParseFunction(std::function<void(std::vector<uint8_t>)> fn) { parseCallback = fn; }
    bool sendData(uint8_t* dataToSend);
private:
    USHORT VID;
    USHORT PID;
    uint8_t collection;
    bool isOpen{ false };        // true if the device is found and open
    GUID hidGuid{ CLSID_NULL };  // pointer to a caller-allocated GUID buffer that the routine uses to return the device interface GUID for HIDClass devices
    HANDLE fileHandle{ INVALID_HANDLE_VALUE };
    std::string VidPid;
    std::string collectionStr;
    static const size_t ReceiveBufferSize = 260;
    static const size_t SendBufferSize = 65;
    const DWORD HidBufferSize = 5; // max 64 bytes of payload including report ID if used (collection not equal 0)
    uint8_t receiveBuffer[ReceiveBufferSize];
    DWORD receivedDataCount;
    OVERLAPPED receiveOverlappedData;
    std::function<void(std::vector<uint8_t>)> parseCallback{ nullptr };
    OVERLAPPED sendOverlappedData;
    DWORD sendDataCount;
    uint8_t sendBuffer[SendBufferSize];
    uint8_t sendErrorCounter{ 0 };
    static const uint8_t SendErrorLimit = 10;
    static const int ConnectionOnPeriod = 5;        //ms
    static const int ConnectionOffPeriod = 100;     //ms
};

