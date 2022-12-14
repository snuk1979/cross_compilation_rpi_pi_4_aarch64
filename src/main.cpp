#include <cstdio> //stdandard output
#include <cstdlib>
#include <string> // std::string
#include <vector> // std::vector<...>
#include <map>    // std::map< ... , ... >
#include <iostream>

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.h>

#include "DeviceManagerRtl.h"

#include "Utility.h"

int main()
try
{
    SoapySDR::logf(SOAPY_SDR_INFO, "*************** Raspberry & SoapySDR & Kraken ***************\n");

    device_manager::CDeviceManagerRtl deviceManager;

    deviceManager.DeviceSearch();

    const auto devCount = deviceManager.GetCountDevice();
    for (size_t numDev = 1; numDev <= devCount; ++numDev)
    {
        deviceManager.SetSampleRate(numDev);
        deviceManager.SetFrequency(numDev);

        deviceManager.PrintDeviceInfo(numDev);
        deviceManager.PrintDeviceSettings(numDev);
    }

    for (size_t numDev = 1; numDev <= devCount; ++numDev)
    {
        deviceManager.StartStream(numDev);
    }

    return EXIT_SUCCESS;
}
catch (const std::runtime_error &error)
{
    LOG_EXP(error.what())
}
