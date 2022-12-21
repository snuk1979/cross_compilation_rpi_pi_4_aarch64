
#include <getopt.h>

#include <SoapySDR/Logger.hpp>
#include <iostream>

#include "DeviceManagerRtl.h"
#include "Utility.h"

int printHelp();

int main(int argc, char* argv[]) try {
    static option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"sample rate", optional_argument, nullptr, 'r'},
        {"frequency", required_argument, nullptr, 'f'},
        {nullptr, no_argument, nullptr, '\0'}};

    double sampleRate = device_manager::CDeviceManagerRtl::kMinSampleRate;
    double frequency = device_manager::CDeviceManagerRtl::kDefFrequency;

    auto long_index = 0;
    auto option = 0;
    while ((option = getopt_long_only(
                argc, argv, "", long_options, &long_index)) != -1) {
        switch (option) {
            case 'h':
                return printHelp();
            case 'r':
                if (nullptr != optarg)
                    sampleRate = std::stod(optarg);
                break;
            case 'f':
                if (nullptr != optarg)
                    frequency = std::stod(optarg);
                break;
        }
    }

    SoapySDR::logf(
        SOAPY_SDR_INFO,
        "*************** Raspberry & SoapySDR & Kraken ***************\n");

    device_manager::CDeviceManagerRtl deviceManager;

    deviceManager.DeviceSearch();

    const auto devCount = deviceManager.GetCountDevice();
    for (size_t numDev = 1; numDev <= devCount; ++numDev) {
        deviceManager.SetSampleRate(sampleRate, numDev);
        deviceManager.SetFrequency(frequency, numDev);

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

int printHelp() {
    std::cout << "Usage SoapySDRUtil [options]" << std::endl;
    std::cout << "  Options summary:" << std::endl;
    std::cout << "    --help \t\t\t\t Print this help message" << std::endl;
    std::cout << "    --rate[=stream rate Sps] \t\t Rate in samples per second"
              << std::endl;
    std::cout << "    --frequency[=specifies\n"
                 "  the down-conversion frequency]\t The center frequency in Hz"
              << std::endl;
    std::cout << std::endl;

    return 0;
}
