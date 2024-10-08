#pragma once

#include <Windows.h>
#include "SimConnect.h"
#include "HidDevice.h"
#include "Arbiter.h"
#include <chrono>
#include <set>
#include <vector>

enum class SimDataFlag : uint8_t
{
    SimDataValid,
    AutopilotOn
};

class Simulator
{
public:
    Simulator(Simulator const&) = delete;
    Simulator& operator=(Simulator const&) = delete;
    static Simulator& getInstance();
    void handler(void);
    static void CALLBACK dispatchWrapper(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext);
    void setHidDevice(HidDevice* pLink) { pHidDeviceLink = pLink; }
    void parseReceivedData(std::vector<uint8_t> receivedData);      // parse received data from HID device link
    void displaySimData();
    void displayReceivedHidDeviceData();
private:
    Simulator();
    ~Simulator();
    void dispatch(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext);    // dispatch messages from SimConnect server
    void subscribe(void);       // subscribes to SimConnect data
    void addToDataDefinition(HANDLE  hSimConnect, SIMCONNECT_DATA_DEFINITION_ID  defineID, std::string datumName, std::string unitsName, SIMCONNECT_DATATYPE  datumType = SIMCONNECT_DATATYPE_FLOAT64);
    void dataRequest(void);     // requests data from SimConnect server
    void requestDataOnSimObject(SIMCONNECT_DATA_REQUEST_ID  RequestID, SIMCONNECT_DATA_DEFINITION_ID  DefineID, SIMCONNECT_PERIOD  Period);
    void procesSimData(SIMCONNECT_RECV* pData);     // processes data received from SimConnect server
    void setSimdataFlag(SimDataFlag flag, bool value);
    void processNewData();  // process received data from SimConnect and prepare for HID device
    HANDLE hSimConnect{ nullptr };
    const uint8_t ShortSleep = 1;
    const uint8_t NormalSleep = 10;
    const uint16_t LongSleep = 1000;
    std::chrono::milliseconds threadSleepTime{ std::chrono::milliseconds(LongSleep) };      // idle time between handler calls
    enum  DataDefineID      // SimConnect data subscription sets
    {
        SimDataReadDefinition,
        SimDataTestDefinition,
        SimDataWriteDefinition
    };
    enum DataRequestID      // SimConnect data request sets
    {
        SimDataReadRequest,
        SimDataTestRequest
    };
    struct SimDataRead      // SimConnect data to be send or compute for HID device
    {
        double aileronPosition;     // used for yoke X zero position calculations (w/o vibratiobs)
        double yokeXindicator;      // used for yoke X zero position calculations (w/o vibratiobs)
        double elevatorTrimPCT;     //used for yoke Y reference position
        double rudderTrimPCT;
        double numberOfEngines;
        double prop1Percent;
        double prop2Percent;
        double estimatedCruiseSpeed;
        double indicatedAirspeed;
        double autopilotMaster;         // is autopilot on?
        double rotationAccBodyX;        // rotation acceleration body X
        double rotationAccBodyY;        // rotation acceleration body Y
        double rotationAccBodyZ;        // rotation acceleration body Z
        double engineType;              // enumerated engine type
        double flapsLeftPosPct;         // percentage of left flaps deflection
        double flapsRightPosPct;        // percentage of right flaps deflection
        double windshieldWindVelocity;  // windshield wind velocity in kts
        double designTakeoffSpeed;      // aircraft design takeoff speed in kts
    };
    struct HidData  // data received from HID device
    {
        float xxx;      // not specified yet
    };
    struct SimDataWriteGen   // general data to set in simulator
    {
        double xxx;
    };
    struct SimDataTest    // SimConnect data for verification and test
    {
        double yokeYposition;
        double yokeYpositionAP;
        double yokeYindicator;
        double elevatorDeflectionPCT;
        double elevatorPosition;
        double elevatorTrimIndicator;
        double elevatorTrimPCT;
    };
    struct SimDataCalculated    // calculated simulation data
    {
        double wingAirSpeed;        // speed referenced to cruise speed
        double flapsPosPct;         // percentage of flaps deflection
        double propellerPct;        // percentage of propeller max rpm
        double stabilizerAirSpeed;  // speed / windshield speed referenced to cruise speed
        double stabTakeoffAirSpeed; // speed / windshield speed referenced to design takeoff speed
    };
    std::set<DWORD> dwIDs;  // set of received SimConnect dwIDs
    HidDevice* pHidDeviceLink{ nullptr };   // pointer to USB HID device
    std::chrono::steady_clock::time_point lastSimDataTime;  // remembers time of last simData reception from server
    std::chrono::steady_clock::time_point lastHidDeviceDataTime;  // remembers time of last HID device data reception
    SimDataRead simDataRead;    // current state of simData
    double simDataInterval{ 0 };    // time between last two simData readouts [s]
    HidData hidData{ 0 };    // data received from HID device
    SimDataWriteGen simDataWriteGen;      // general data to be written to simulator
    std::chrono::steady_clock::time_point lastHidDeviceSendTime;  // remembers time of last HID device data sending
    static const size_t HidSendBufferSize = 64;
    uint8_t hidSendBuffer[HidSendBufferSize];
    uint32_t simDataFlags{ 0 };     //bit flags received from simulator
    bool simConnectSetError{ false };   //last attempt to set in SimConnect failed?
    bool simConnectResponseError{ false };  //last connection to SimConnect failed?
    SimDataCalculated simDataCalculated;    // calculated parameters
};

