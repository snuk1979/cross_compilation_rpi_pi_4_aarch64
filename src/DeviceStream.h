#ifndef __DEVICE_STREAM_H__
#define __DEVICE_STREAM_H__

#include <SoapySDR/Types.hpp>
#include <memory>
#include <string>
#include <vector>

#include "DataQueue.h"

namespace SoapySDR {
class Device;
class Stream;
}  // namespace SoapySDR

namespace device_stream {
class IDeviceStream {
 public:
  virtual void RunStreamLoop(
      data_queue::RawQueue& dataQueue,
      std::shared_ptr<SoapySDR::Device> device,
      const int direction,
      const std::string& format,
      const std::vector<size_t>& channels = std::vector<size_t>(),
      const SoapySDR::Kwargs& args = SoapySDR::Kwargs()) = 0;

  virtual ~IDeviceStream(){};
};

}  // namespace device_stream

#endif  // __DEVICE_STREAM_H__
