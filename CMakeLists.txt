cmake_minimum_required(VERSION 3.20.0)
project(SimConnectClient VERSION 0.0.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(MSFS_SDK "C:/MSFS SDK")

set(PROJECT_NAME SimConnectClient)

add_executable(${PROJECT_NAME} 
    src/SimConnectClient.cpp
    src/Simulator.cpp
    src/HidDevice.cpp
    src/Console.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    "_ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH"
    "_DEBUG"
)

set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")

target_include_directories(${PROJECT_NAME} PRIVATE
    "${MSFS_SDK}/SimConnect SDK/include"
)

target_link_directories(${PROJECT_NAME} PRIVATE
    "${MSFS_SDK}/SimConnect SDK/lib/static"
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    "${MSFS_SDK}/SimConnect SDK/lib/static/SimConnect_debug.lib"
    shlwapi.lib
    user32.lib
    Ws2_32.lib
)


