#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include <memory>

#include "DeviceManager.h"

namespace SoapySDR {
class Device;
}  // namespace SoapySDR
namespace device_manager {
class CDeviceManagerRtl : public IDeviceManager {
 public:
  CDeviceManagerRtl();
  CDeviceManagerRtl(const CDeviceManagerRtl&) = delete;
  CDeviceManagerRtl& operator=(const CDeviceManagerRtl&) = delete;
  CDeviceManagerRtl(CDeviceManagerRtl&&);
  CDeviceManagerRtl& operator=(CDeviceManagerRtl&&);
  ~CDeviceManagerRtl() override;

  bool DeviceSearch() override;

  bool SetSampleRate(const double rate = kMinSampleRate,
                     const int deviceNumber = 1,
                     const int direction = SOAPY_SDR_RX,
                     const size_t channel = 0u) override;

  bool SetFrequency(const double value = kDefFrequency,
                    const int deviceNumber = 1,
                    const int direction = SOAPY_SDR_RX,
                    const size_t channel = 0u,
                    const SoapySDR::Kwargs& args = SoapySDR::Kwargs()) override;

  bool StartStream(const int deviceNumber = 1,
                   const int direction = SOAPY_SDR_RX,
                   const std::string& format = "",
                   const std::vector<size_t>& channels = std::vector<size_t>(1,
                                                                             0),
                   const SoapySDR::Kwargs& args = SoapySDR::Kwargs()) override;

  void StopStreams() override;

  void WaitShutdownSignal() override;

  std::size_t GetCountDevice() const override;

  SoapySDR::Kwargs GetHardwareInfo(const int deviceNumber = 0) const override;

  void PrintDeviceInfo(const int deviceNumber) const override;

  void PrintDeviceSettings(const int deviceNumber) const override;

 private:
  bool AddDevice(const SoapySDR::Kwargs& args);
  std::shared_ptr<SoapySDR::Device> GetDevice(const int deviceNumber) const;
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

}  // namespace device_manager

#endif  //__DEVICE_MANAGER_H__
