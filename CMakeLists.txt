cmake_minimum_required(VERSION 3.20.0)
project(SimConnectClient VERSION 0.0.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(MSFS_SDK "C:/MSFS SDK")

add_executable(SimConnectClient 
    src/SimConnectClient.cpp
    src/Simulator.cpp
    src/HidDevice.cpp
    src/Console.cpp
)

target_include_directories(SimConnectClient PRIVATE
    "${MSFS_SDK}/SimConnect SDK/include"
)

target_link_directories(SimConnectClient PRIVATE
    "${MSFS_SDK}/SimConnect SDK/lib"
)

target_link_libraries(SimConnectClient PUBLIC
    "${MSFS_SDK}/SimConnect SDK/lib/SimConnect.lib"
    shlwapi.lib
    user32.lib
    Ws2_32.lib
)

# add_dependencies(SimConnectClient
#     "${MSFS_SDK}/SimConnect SDK/lib/SimConnect.lib"
# )