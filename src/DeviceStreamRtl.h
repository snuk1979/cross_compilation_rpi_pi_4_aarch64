#ifndef __DEVICE_STREAMRTL_H__
#define __DEVICE_STREAMRTL_H__

#include "DeviceStream.h"

#include <SoapySDR/Types.hpp>
#include <SoapySDR/Errors.hpp>

namespace device_stream
{

    class CDeviceStreamRtl : public IDeviceStream
    {
    public:
        CDeviceStreamRtl();
        CDeviceStreamRtl(const CDeviceStreamRtl &) = delete;
        CDeviceStreamRtl(CDeviceStreamRtl &&);
        CDeviceStreamRtl &operator=(const CDeviceStreamRtl &) = delete;
        CDeviceStreamRtl &operator=(CDeviceStreamRtl &&) = delete;
        ~CDeviceStreamRtl() override;

        void RunStreamLoop(
            std::shared_ptr<SoapySDR::Device> device,
            const int direction,
            const std::string &format,
            const std::vector<size_t> &channels = std::vector<size_t>(1, 0),
            const SoapySDR::Kwargs &args = SoapySDR::Kwargs()) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };

} // device_stream

#endif // __DEVICE_STREAMRTL_H__
