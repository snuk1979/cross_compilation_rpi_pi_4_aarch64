#include <cstdio> //stdandard output
#include <cstdlib>

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.h>

#include <string> // std::string
#include <vector> // std::vector<...>
#include <map>    // std::map< ... , ... >

#include <iostream>

constexpr auto kMinSampleRate = 30.72e6;
constexpr auto kSizeBuff = 1024;
#define LOG_EXP(WHAT) SoapySDR::logf(SOAPY_SDR_FATAL, "*************** Exception: %s %s", WHAT, "***************\n");

int main()
try
{
    SoapySDR::logf(SOAPY_SDR_INFO, "*************** Raspberry & SoapySDR & Kraken ***************\n");

    // 0. enumerate devices (list all devices' information)
    SoapySDR::KwargsList results = SoapySDR::Device::enumerate();
    SoapySDR::Kwargs::iterator it;

    printf("\n");

    for (size_t i = 0; i < results.size(); ++i)
    {
        SoapySDR::logf(SOAPY_SDR_INFO, "Found device #%d: ", i);
        for (it = results[i].begin(); it != results[i].end(); ++it)
        {
            SoapySDR::logf(SOAPY_SDR_INFO, "%s = %s", it->first.c_str(), it->second.c_str());
        }
        printf("\n");
    }

    if (results.empty())
    {
        SoapySDR::logf(SOAPY_SDR_ERROR, "Device isn't found, no work to do");
        return EXIT_FAILURE;
    }

    // 1. create device instance

    //	1.1 set arguments
    //		args can be user defined or from the enumeration result
    //		We use first results as args here:
    SoapySDR::Kwargs args = results[0];

    //	1.2 make device
    SoapySDR::Device *sdr = SoapySDR::Device::make(args);

    if (sdr == NULL)
    {
        SoapySDR::logf(SOAPY_SDR_FATAL, "SoapySDR::Device::make failed\n");
        return EXIT_FAILURE;
    }

    // 2. query device info
    std::vector<std::string> str_list; // string list

    //	2.1 antennas
    str_list = sdr->listAntennas(SOAPY_SDR_RX, 0);
    auto info_buf_str = std::string("Rx antennas: ");
    for (size_t i = 0; i < str_list.size(); ++i)
    {
        info_buf_str += str_list[i] + ",";
    }
    SoapySDR::logf(SOAPY_SDR_INFO, "%s", info_buf_str.c_str());

    //	2.2 gains
    str_list = sdr->listGains(SOAPY_SDR_RX, 0);
    info_buf_str = "Rx Gains: ";
    for (size_t i = 0; i < str_list.size(); ++i)
    {
        info_buf_str += str_list[i] + ", ";
    }
    SoapySDR::logf(SOAPY_SDR_INFO, "%s", info_buf_str.c_str());

    //	2.3. ranges(frequency ranges)
    SoapySDR::RangeList ranges = sdr->getFrequencyRange(SOAPY_SDR_RX, 0);
    info_buf_str = "Rx freq ranges: ";
    for (size_t i = 0; i < ranges.size(); ++i)
    {
        info_buf_str += "[" + std::to_string(ranges[i].minimum()) + " Hz -> " + std::to_string(ranges[i].maximum()) + " Hz], ";
    }
    SoapySDR::logf(SOAPY_SDR_INFO, "%s", info_buf_str.c_str());

    // 3. apply settings
    try
    {
        auto sample_rate_range = sdr->getSampleRateRange(SOAPY_SDR_RX, 0);
        for (const auto &range : sample_rate_range)
        {
            SoapySDR::logf(SOAPY_SDR_INFO, "minimum: %f maximum: %f step: %f", range.minimum(), range.maximum(), range.step());
        }

        auto minSampleRate = sample_rate_range[0].maximum() < kMinSampleRate ? sample_rate_range[0].maximum() : kMinSampleRate;

        SoapySDR::logf(SOAPY_SDR_INFO, "Setting sample rate to %f", minSampleRate);

        sdr->setSampleRate(SOAPY_SDR_RX, 0, minSampleRate);
    }
    catch (const std::runtime_error &error)
    {
        SoapySDR::logf(SOAPY_SDR_ERROR, ": %s", error.what());
    }

    sdr->setFrequency(SOAPY_SDR_RX, 0, 433e6);

    // 4. setup a stream (complex floats)
    SoapySDR::Stream *rx_stream = sdr->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
    if (rx_stream == NULL)
    {
        SoapySDR::logf(SOAPY_SDR_FATAL, "Setup Stream failed");
        SoapySDR::Device::unmake(sdr);
        return EXIT_FAILURE;
    }
    sdr->activateStream(rx_stream, 0, 0, 0);

    // 5. create a re-usable buffer for rx samples
    std::complex<float> data_buff[kSizeBuff] = {0};

    // 6. receive some samples
    for (int i = 0; i < 10; ++i)
    {
        void *buffs[] = {data_buff};
        int flags;
        long long time_ns;
        int retCode = sdr->readStream(rx_stream, buffs, kSizeBuff, flags, time_ns);
        SoapySDR::logf(SOAPY_SDR_INFO, "retCode = %d, flags = %d, time_ns = %lld", retCode, flags, time_ns);

        if (retCode < 0)
        {
            SoapySDR::logf(SOAPY_SDR_ERROR, "%s", SoapySDR_errToStr(retCode));
            continue;
        }

        int numVal = -1;
        for (const auto &val : data_buff)
        {
            SoapySDR::logf(SOAPY_SDR_INFO, "index[%d] (%f,%f)", ++numVal, val.real(), val.imag());
        }
    }

    // 7. shutdown the stream
    sdr->deactivateStream(rx_stream, 0, 0); // stop streaming
    sdr->closeStream(rx_stream);

    // 8. cleanup device handle
    SoapySDR::Device::unmake(sdr);
    SoapySDR::logf(SOAPY_SDR_INFO, "Done");

    return EXIT_SUCCESS;
}
catch (const std::runtime_error &error)
{
    LOG_EXP(error.what())
}