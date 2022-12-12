#include "DeviceManagerRtl.h"

#include <vector>
#include <atomic>

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.h>

#define LOG_FUNC() SoapySDR::logf(SOAPY_SDR_INFO, "%s", __PRETTY_FUNCTION__)

namespace device_manager
{
    constexpr auto kDeviceIdent = "serial";

    struct DeviceData
    {
        DeviceData(std::shared_ptr<SoapySDR::Device> device, const SoapySDR::Kwargs &args) : mDevice(std::move(device)), mArgs(args), mStopStreamFlag(false)
        {
        }

        DeviceData(DeviceData &&rh) : mDevice(std::move(rh.mDevice)), mArgs(std::move(rh.mArgs)), mStopStreamFlag(std::move(rh.mStopStreamFlag))
        {
        }

        std::shared_ptr<SoapySDR::Device> mDevice;
        const SoapySDR::Kwargs mArgs;
        volatile std::atomic_bool mStopStreamFlag;
    };

    struct CDeviceManagerRtl::Impl
    {
        const DeviceData *GetDeviceData(const int deviceNumber) const;

        std::vector<DeviceData> mDeviceStorage;
        std::mutex mLock;
    };

    const DeviceData *CDeviceManagerRtl::Impl::GetDeviceData(const int deviceNumber) const
    {
        static constexpr auto kMinDevNumber = 1;

        if (kMinDevNumber > deviceNumber || static_cast<std::size_t>(deviceNumber) > mDeviceStorage.size())
        {
            SoapySDR::logf(SOAPY_SDR_ERROR, " Device number not valid, valid range: %d - %u", kMinDevNumber, mDeviceStorage.size());
            return nullptr;
        }

        return &mDeviceStorage[deviceNumber - 1];
    }

    CDeviceManagerRtl::CDeviceManagerRtl() : mImpl(new CDeviceManagerRtl::Impl)
    {
        LOG_FUNC();
    }

    CDeviceManagerRtl::CDeviceManagerRtl(CDeviceManagerRtl &&) = default;

    CDeviceManagerRtl &CDeviceManagerRtl::operator=(CDeviceManagerRtl &&) = default;

    CDeviceManagerRtl::~CDeviceManagerRtl() = default;

    bool CDeviceManagerRtl::DeviceSearch()
    {
        LOG_FUNC();

        // 0. enumerate devices (list all devices' information)
        const auto devicesArgsList = SoapySDR::Device::enumerate();

        if (devicesArgsList.empty())
        {
            SoapySDR::logf(SOAPY_SDR_ERROR, "Devices aren't found, no work to do");
            return false;
        }

        SoapySDR::logf(SOAPY_SDR_INFO, "**** Number of devices found: %u ****", devicesArgsList.size());

        for (const auto &args : devicesArgsList)
        {
            CallThreadSafe(mImpl->mLock, this, &CDeviceManagerRtl::AddDevice, args);
        }

        return 0u != CallThreadSafe(mImpl->mLock, this, &CDeviceManagerRtl::GetCountDevice);
    }

    bool CDeviceManagerRtl::SetSampleRate(
        const int deviceNumber,
        const int direction,
        const size_t channel,
        const double rate)
    {
        LOG_FUNC();

        if (auto device = CallThreadSafe(mImpl->mLock, this, &CDeviceManagerRtl::GetDevice, deviceNumber))
        {
            try
            {
                auto minSampleRate = rate;

                const auto sampleRateRange = device->getSampleRateRange(direction, channel);
                for (const auto &range : sampleRateRange)
                {
                    SoapySDR::logf(SOAPY_SDR_INFO, "Direction: %d Channel: %u minimum: %f maximum: %f step: %f",
                                   direction, channel, range.minimum(), range.maximum(), range.step());

                    minSampleRate = range.maximum() < minSampleRate ? range.maximum() : minSampleRate;
                }

                SoapySDR::logf(SOAPY_SDR_INFO, "Setting sample rate to %f", minSampleRate);

                device->setSampleRate(direction, channel, minSampleRate);

                return true;
            }
            catch (const std::runtime_error &error)
            {
                SoapySDR::logf(SOAPY_SDR_ERROR, ": %s", error.what());
            }
        }

        return false;
    }

    bool CDeviceManagerRtl::SetFrequency(
        const int deviceNumber,
        const int direction,
        const size_t channel,
        const double value,
        [[maybe_unused]] const SoapySDR::Kwargs &args)
    {
        LOG_FUNC();

        if (auto device = CallThreadSafe(mImpl->mLock, this, &CDeviceManagerRtl::GetDevice, deviceNumber))
        {
            device->setFrequency(direction, channel, value);
            return true;
        }

        return false;
    }

    bool CDeviceManagerRtl::StartStream(
        [[maybe_unused]] const int deviceNumber,
        [[maybe_unused]] const int direction,
        [[maybe_unused]] const std::string &format,
        [[maybe_unused]] const std::vector<size_t> &channels,
        [[maybe_unused]] const SoapySDR::Kwargs &args)
    {
        LOG_FUNC();

        return false;
    }

    bool CDeviceManagerRtl::StopStream(const int deviceNumber)
    {
        LOG_FUNC();
        (void)(deviceNumber);
        return false;
    }

    std::size_t CDeviceManagerRtl::GetCountDevice() const
    {
        return mImpl->mDeviceStorage.size();
    }

    SoapySDR::Kwargs CDeviceManagerRtl::GetHardwareInfo(const int deviceNumber) const
    {
        if (const auto deviceData = mImpl->GetDeviceData(deviceNumber))
        {
            return deviceData->mArgs;
        }

        return SoapySDR::Kwargs();
    }

    void CDeviceManagerRtl::PrintDeviceInfo(const int deviceNumber) const
    {
        SoapySDR::logf(SOAPY_SDR_NOTICE, "Device #%d: ", deviceNumber);

        auto args = CallThreadSafe(mImpl->mLock, this, &CDeviceManagerRtl::GetHardwareInfo, deviceNumber);

        if (!args.empty())
        {
            args.merge(CallThreadSafe(mImpl->mLock, this, &CDeviceManagerRtl::GetDevice, deviceNumber)->getHardwareInfo());
        }

        for (const auto &info : args)
        {
            SoapySDR::logf(SOAPY_SDR_INFO, "%s = %s", info.first.c_str(), info.second.c_str());
        }
    }

    void CDeviceManagerRtl::PrintDeviceSettings(const int deviceNumber) const
    {
        const auto device = CallThreadSafe(mImpl->mLock, this, &CDeviceManagerRtl::GetDevice, deviceNumber);
        if (!device)
        {
            return;
        }

        //	1 antennas
        auto strBuff = std::string("Rx antennas: ");
        for (const auto &antennaName : device->listAntennas(SOAPY_SDR_RX, 0))
        {
            strBuff += antennaName + ",";
        }
        SoapySDR::logf(SOAPY_SDR_INFO, "%s", strBuff.c_str());

        //	1.1 gains
        strBuff = "Rx Gains: ";
        for (const auto &gainName : device->listGains(SOAPY_SDR_RX, 0))
        {
            strBuff += gainName + ", ";
        }
        SoapySDR::logf(SOAPY_SDR_INFO, "%s", strBuff.c_str());

        //	1.2 ranges(frequency ranges)
        strBuff = "Rx freq ranges: ";
        for (const auto &freqRange : device->getFrequencyRange(SOAPY_SDR_RX, 0))
        {
            strBuff += "[" + std::to_string(freqRange.minimum()) + " Hz -> " + std::to_string(freqRange.maximum()) + " Hz], ";
        }
        SoapySDR::logf(SOAPY_SDR_INFO, "%s", strBuff.c_str());
    }

    bool CDeviceManagerRtl::AddDevice(const SoapySDR::Kwargs &args)
    {
        LOG_FUNC();

        const auto it = args.find(kDeviceIdent);

        const auto deviceIdent = it != args.end() ? it->second : "Undifine";

        // Create device instance

        auto device = SoapySDR::Device::make(args);

        if (nullptr != device)
        {
            auto deleter = [devIdent = deviceIdent](SoapySDR::Device *device)
            {
                if (nullptr == device)
                {
                    SoapySDR::logf(SOAPY_SDR_NOTICE, "Unmake device: %s - pointer is nullptr", devIdent);
                    return;
                }

                SoapySDR::logf(SOAPY_SDR_NOTICE, "Unmake device: %s", devIdent.c_str());
                SoapySDR::Device::unmake(device);
            };

            SoapySDR::logf(SOAPY_SDR_NOTICE, "Device %s made", deviceIdent.c_str());

            mImpl->mDeviceStorage.emplace_back(std::shared_ptr<SoapySDR::Device>(device, deleter), args);

            return true;
        }

        SoapySDR::logf(SOAPY_SDR_FATAL, "SoapySDR::Device::make %s failed", deviceIdent.c_str());

        return false;
    }

    std::shared_ptr<SoapySDR::Device> CDeviceManagerRtl::GetDevice(const int deviceNumber) const
    {
        if (const auto deviceData = mImpl->GetDeviceData(deviceNumber))
        {
            return deviceData->mDevice;
        }

        return nullptr;
    }

} // namespace device_manager
