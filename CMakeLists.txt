cmake_minimum_required(VERSION 3.10.0)
project(splav-encoder-read-cpp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBMODBUS REQUIRED libmodbus)

add_executable(splav-encoder-read-cpp main.cpp 
    Channel5Data.h
    MessageSender.h MessageSender.cpp
    ModbusServer.h ModbusServer.cpp
    ModbusRegistersView.h
    HardwareManager.h HardwareManager.cpp)

target_include_directories(splav-encoder-read-cpp PRIVATE ${LIBMODBUS_INCLUDE_DIRS})
target_link_libraries(splav-encoder-read-cpp ${LIBMODBUS_LIBRARIES})

target_link_libraries(splav-encoder-read-cpp pigpio)



