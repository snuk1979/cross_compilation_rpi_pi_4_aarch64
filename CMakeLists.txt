cmake_minimum_required(VERSION 3.16)

project(KrakenSDR CXX)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} main.cpp)

find_package(SoapySDR CONFIG)

if (NOT SoapySDR_FOUND)
    message(WARNING "SoapySDR development files not found - skipping support")
    return()
endif ()

target_link_libraries(${PROJECT_NAME} SoapySDR)
