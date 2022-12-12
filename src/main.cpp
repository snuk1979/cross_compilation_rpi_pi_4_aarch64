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

constexpr auto kMinSampleRate = 30.72e6;
constexpr auto kSizeBuff = 1024;
#define LOG_EXP(WHAT) SoapySDR::logf(SOAPY_SDR_FATAL, "*************** Exception: %s %s", WHAT, "***************\n");

int main()
try
{
    SoapySDR::logf(SOAPY_SDR_INFO, "*************** Raspberry & SoapySDR & Kraken ***************\n");

    device_manager::CDeviceManagerRtl deviceManager;

    deviceManager.DeviceSearch();

    const auto devCount = deviceManager.GetCountDevice();
    for (size_t numDev = 1; numDev <= devCount; ++numDev)
    {
        deviceManager.PrintDeviceInfo(numDev);
        deviceManager.PrintDeviceSettings(numDev);
    }

    // // 4. setup a stream (complex floats)
    // SoapySDR::Stream *rx_stream = sdr->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
    // if (rx_stream == NULL)
    // {
    //     SoapySDR::logf(SOAPY_SDR_FATAL, "Setup Stream failed");
    //     SoapySDR::Device::unmake(sdr);
    //     return EXIT_FAILURE;
    // }
    // sdr->activateStream(rx_stream, 0, 0, 0);

    // // 5. create a re-usable buffer for rx samples
    // std::complex<float> data_buff[kSizeBuff] = {0};

    // // 6. receive some samples
    // for (int i = 0; i < 10; ++i)
    // {
    //     void *buffs[] = {data_buff};
    //     int flags;
    //     long long time_ns;
    //     int retCode = sdr->readStream(rx_stream, buffs, kSizeBuff, flags, time_ns);
    //     SoapySDR::logf(SOAPY_SDR_INFO, "retCode = %d, flags = %d, time_ns = %lld", retCode, flags, time_ns);

    //     if (retCode < 0)
    //     {
    //         SoapySDR::logf(SOAPY_SDR_ERROR, "%s", SoapySDR_errToStr(retCode));
    //         continue;
    //     }

    //     int numVal = -1;
    //     for (const auto &val : data_buff)
    //     {
    //         SoapySDR::logf(SOAPY_SDR_INFO, "index[%d] (%f,%f)", ++numVal, val.real(), val.imag());
    //     }
    // }

    // // 7. shutdown the stream
    // sdr->deactivateStream(rx_stream, 0, 0); // stop streaming
    // sdr->closeStream(rx_stream);

    // // 8. cleanup device handle
    // SoapySDR::Device::unmake(sdr);
    // SoapySDR::logf(SOAPY_SDR_INFO, "Done");

    return EXIT_SUCCESS;
}
catch (const std::runtime_error &error)
{
    LOG_EXP(error.what())
}