#include "DeviceStreamRtl.h"

#include <cstdlib>
#include <stdexcept>
#include <csignal>
#include <chrono>
#include <cstdio>
#include <thread>

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>

#include "Utility.h"

sig_atomic_t streamLoopDone = false;
static void sigIntHandler(const int)
{
    LOG_FUNC();
    streamLoopDone = true;
}

namespace device_stream
{
    struct CStreamDeleter
    {
        CStreamDeleter(std::weak_ptr<SoapySDR::Device> device) : mDevice(std::move(device)) {}
        void operator()(SoapySDR::Stream *stream)
        {
            LOG_FUNC();

            if (auto device = mDevice.lock())
            {
                // cleanup stream and device
                SoapySDR::logf(SOAPY_SDR_NOTICE, "Deactivate & close Stream: %u", stream);
                device->deactivateStream(stream);
                device->closeStream(stream);
            }
        }

        std::weak_ptr<SoapySDR::Device> mDevice;
    };
    struct CDeviceStreamRtl::Impl
    {
        Impl() : mStream(nullptr, CStreamDeleter(std::weak_ptr<SoapySDR::Device>())) {}

        ~Impl()
        {
            LOG_FUNC();

            if (mThreadLoop.joinable())
            {
                mThreadLoop.join();
            }
        }

        void SetupStream(
            std::shared_ptr<SoapySDR::Device> device,
            const int direction,
            const std::string &format,
            const std::vector<size_t> &channels,
            const SoapySDR::Kwargs &args = SoapySDR::Kwargs());

        static void StreamLoop(
            std::shared_ptr<SoapySDR::Device> device,
            std::unique_ptr<SoapySDR::Stream, CStreamDeleter> stream,
            const int direction,
            const size_t numChans,
            const size_t elemSize);

        std::unique_ptr<SoapySDR::Stream, CStreamDeleter> mStream;
        std::thread mThreadLoop;
    };

    CDeviceStreamRtl::CDeviceStreamRtl() : mImpl(std::make_unique<CDeviceStreamRtl::Impl>())
    {
    }

    CDeviceStreamRtl::CDeviceStreamRtl(CDeviceStreamRtl &&) = default;

    CDeviceStreamRtl::~CDeviceStreamRtl()
    {
        LOG_FUNC();
    }

    void CDeviceStreamRtl::RunStreamLoop(
        std::shared_ptr<SoapySDR::Device> device,
        const int direction,
        const std::string &format,
        const std::vector<size_t> &channels,
        const SoapySDR::Kwargs &args)
    {
        LOG_FUNC();

        mImpl->SetupStream(std::move(device), direction, format, channels, args);
    }

    void CDeviceStreamRtl::Impl::SetupStream(
        std::shared_ptr<SoapySDR::Device> device,
        const int direction,
        const std::string &formatStr,
        const std::vector<size_t> &channels,
        [[maybe_unused]] const SoapySDR::Kwargs &args)
    {
        LOG_FUNC();

        // create the stream, use the native format
        double fullScale(0.0);
        const auto format = formatStr.empty() ? device->getNativeStreamFormat(direction, channels.front(), fullScale) : formatStr;
        const auto elemSize = SoapySDR::formatToSize(format);
        auto stream = device->setupStream(direction, format, channels);

        SoapySDR::logf(SOAPY_SDR_NOTICE, "setupStream: %u", stream);

        SoapySDR::logf(SOAPY_SDR_INFO, "Stream format: %s", format.c_str());
        SoapySDR::logf(SOAPY_SDR_INFO, "Num channels: %u", channels.size());
        SoapySDR::logf(SOAPY_SDR_INFO, "Element size: %u", elemSize);
        SoapySDR::logf(SOAPY_SDR_INFO, "Begin SOAPY_SDR_RX rate test at %f  Msps", device->getSampleRate(SOAPY_SDR_RX, channels.front()) / 1e6);

        auto streamUPtr = std::unique_ptr<SoapySDR::Stream, CStreamDeleter>(stream, CStreamDeleter(device));

        mThreadLoop = std::thread(StreamLoop, device, std::move(streamUPtr), direction, channels.size(), elemSize);
    }

    void CDeviceStreamRtl::Impl::StreamLoop(
        std::shared_ptr<SoapySDR::Device> device,
        std::unique_ptr<SoapySDR::Stream, CStreamDeleter> stream,
        const int direction,
        const size_t numChans,
        const size_t elemSize)
    {
        LOG_FUNC();

        // allocate buffers for the stream read/write
        const auto numElems = device->getStreamMTU(stream.get());
        std::vector<std::vector<char>> buffMem(numChans, std::vector<char>(elemSize * numElems));
        std::vector<void *> buffs(numChans);
        for (size_t i = 0; i < numChans; i++)
        {
            buffs[i] = buffMem[i].data();
        }

        // state collected in this loop
        unsigned int overflows(0);
        unsigned int underflows(0);
        unsigned long long totalSamples(0);

        const auto startTime = std::chrono::high_resolution_clock::now();
        auto timeLastPrint = std::chrono::high_resolution_clock::now();
        auto timeLastSpin = std::chrono::high_resolution_clock::now();
        auto timeLastStatus = std::chrono::high_resolution_clock::now();
        int spinIndex(0);

        SoapySDR::logf(SOAPY_SDR_INFO, "Starting stream loop, press Ctrl+C to exit...");
        device->activateStream(stream.get());
        signal(SIGINT, sigIntHandler);
        while (not streamLoopDone)
        {
            int ret(0);
            int flags(0);
            long long timeNs(0);
            switch (direction)
            {
            case SOAPY_SDR_RX:
                ret = device->readStream(stream.get(), buffs.data(), numElems, flags, timeNs);
                break;
            case SOAPY_SDR_TX:
                ret = device->writeStream(stream.get(), buffs.data(), numElems, flags, timeNs);
                break;
            }

            if (SOAPY_SDR_TIMEOUT == ret)
                continue;
            if (SOAPY_SDR_OVERFLOW == ret)
            {
                overflows++;
                continue;
            }
            if (SOAPY_SDR_UNDERFLOW == ret)
            {
                underflows++;
                continue;
            }
            if (ret < 0)
            {
                SoapySDR::logf(SOAPY_SDR_ERROR, "Unexpected stream error %s", SoapySDR::errToStr(ret));
                break;
            }
            totalSamples += ret;

            const auto now = std::chrono::high_resolution_clock::now();
            if (timeLastSpin + std::chrono::milliseconds(300) < now)
            {
                timeLastSpin = now;
                static const char spin[] = {"|/-\\"};
                printf("\b%c", spin[(spinIndex++) % 4]);
                fflush(stdout);
                // SoapySDR::logf(SOAPY_SDR_INFO, "\b%c", spin[(spinIndex++) % 4]);
            }
            // occasionally read out the stream status (non blocking)
            if (timeLastStatus + std::chrono::seconds(1) < now)
            {
                timeLastStatus = now;
                while (true)
                {
                    size_t chanMask;
                    int flags;
                    long long timeNs;
                    ret = device->readStreamStatus(stream.get(), chanMask, flags, timeNs, 0);
                    if (SOAPY_SDR_OVERFLOW == ret)
                        overflows++;
                    else if (SOAPY_SDR_UNDERFLOW == ret)
                        underflows++;
                    else if (SOAPY_SDR_TIME_ERROR == ret)
                    {
                    }
                    else
                        break;
                }
            }
            if (timeLastPrint + std::chrono::seconds(5) < now)
            {
                timeLastPrint = now;
                const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
                const auto sampleRate = double(totalSamples) / timePassed.count();
                printf("\b%g Msps\t%g MBps", sampleRate, sampleRate * numChans * elemSize);
                if (0u != overflows)
                    printf("\tOverflows %u", overflows);
                if (0u != underflows)
                    printf("\tUnderflows %u", underflows);
                if(0u < totalSamples)
                {
                    printf("\tTotalSamples %llu", totalSamples);
                }
                printf("\n ");
                // SoapySDR::logf(SOAPY_SDR_INFO, "\b%g Msps\t%g MBps", sampleRate, sampleRate * numChans * elemSize);
                // if (overflows != 0)
                //     SoapySDR::logf(SOAPY_SDR_INFO, "\tOverflows %u", overflows);
                // if (underflows != 0)
                //     SoapySDR::logf(SOAPY_SDR_INFO, "\tUnderflows %u", underflows);
            }
        }
    }

} // namespace device_stream
