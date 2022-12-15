#include <SoapySDR/Errors.h>

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Types.hpp>
#include <cstdio>  //stdandard output
#include <cstdlib>
#include <iostream>
#include <map>     // std::map< ... , ... >
#include <string>  // std::string
#include <vector>  // std::vector<...>

#include "DeviceManagerRtl.h"
#include "Utility.h"

int main() try {
  SoapySDR::logf(
      SOAPY_SDR_INFO,
      "*************** Raspberry & SoapySDR & Kraken ***************\n");

  device_manager::CDeviceManagerRtl deviceManager;

  deviceManager.DeviceSearch();

  const auto devCount = deviceManager.GetCountDevice();
  for (size_t numDev = 1; numDev <= devCount; ++numDev) {
    deviceManager.SetSampleRate(numDev);
    deviceManager.SetFrequency(numDev);

    deviceManager.PrintDeviceInfo(numDev);
    deviceManager.PrintDeviceSettings(numDev);
  }

  for (size_t numDev = 1; numDev <= devCount; ++numDev) {
    deviceManager.StartStream(numDev);
  }

  deviceManager.WaitShutdownSignal();

  return EXIT_SUCCESS;
} catch (const std::runtime_error& error) {
  LOG_EXP(error.what())
}
