#include "DeviceStreamRtl.h"

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <future>
#include <stdexcept>

#include "Utility.h"

sig_atomic_t streamLoopDone = false;
static void sigIntHandler(const int) {
  LOG_FUNC();
  streamLoopDone = true;
}

namespace device_stream {
struct CStreamDeleter {
  CStreamDeleter(std::weak_ptr<SoapySDR::Device> device)
      : mDevice(std::move(device)) {}
  void operator()(SoapySDR::Stream* stream) {
    LOG_FUNC();

    if (auto device = mDevice.lock()) {
      // cleanup stream and device
      SoapySDR::logf(SOAPY_SDR_NOTICE, "Deactivate & close Stream: %p", stream);
      device->deactivateStream(stream);
      device->closeStream(stream);
    }
  }

  std::weak_ptr<SoapySDR::Device> mDevice;
};
struct CDeviceStreamRtl::Impl {
  ~Impl() {
    LOG_FUNC();

    if (mThreadHandle.valid()) {
      const auto msg = mThreadHandle.get();
      SoapySDR::logf(SOAPY_SDR_INFO, "%s", msg.c_str());
    }
  }

  void SetupStream(data_queue::RawQueue& dataQueue,
                   std::shared_ptr<SoapySDR::Device> device,
                   const int direction,
                   const std::string& format,
                   const std::vector<size_t>& channels,
                   const SoapySDR::Kwargs& args = SoapySDR::Kwargs());

  static std::string StreamLoop(
      data_queue::RawQueue& dataQueue,
      std::shared_ptr<SoapySDR::Device> device,
      std::unique_ptr<SoapySDR::Stream, CStreamDeleter> stream,
      const int direction,
      const size_t numChans,
      const size_t elemSize);

  std::future<std::string> mThreadHandle;
};

CDeviceStreamRtl::CDeviceStreamRtl()
    : mImpl(std::make_unique<CDeviceStreamRtl::Impl>()) {}

CDeviceStreamRtl::CDeviceStreamRtl(CDeviceStreamRtl&&) = default;

CDeviceStreamRtl::~CDeviceStreamRtl() {
  LOG_FUNC();
}

void CDeviceStreamRtl::RunStreamLoop(data_queue::RawQueue& dataQueue,
                                     std::shared_ptr<SoapySDR::Device> device,
                                     const int direction,
                                     const std::string& format,
                                     const std::vector<size_t>& channels,
                                     const SoapySDR::Kwargs& args) {
  LOG_FUNC();

  mImpl->SetupStream(
      dataQueue, std::move(device), direction, format, channels, args);
}

void CDeviceStreamRtl::Impl::SetupStream(
    data_queue::RawQueue& dataQueue,
    std::shared_ptr<SoapySDR::Device> device,
    const int direction,
    const std::string& formatStr,
    const std::vector<size_t>& channels,
    [[maybe_unused]] const SoapySDR::Kwargs& args) {
  LOG_FUNC();

  // create the stream, use the native format
  double fullScale(0.0);
  const auto format = formatStr.empty()
                          ? device->getNativeStreamFormat(
                                direction, channels.front(), fullScale)
                          : formatStr;
  const auto elemSize = SoapySDR::formatToSize(format);
  auto stream = device->setupStream(direction, format, channels);

  SoapySDR::logf(SOAPY_SDR_NOTICE, "setupStream: %p", stream);

  SoapySDR::logf(SOAPY_SDR_INFO, "Stream format: %s", format.c_str());
  SoapySDR::logf(SOAPY_SDR_INFO, "Num channels: %u", channels.size());
  SoapySDR::logf(SOAPY_SDR_INFO, "Element size: %u", elemSize);
  SoapySDR::logf(SOAPY_SDR_INFO,
                 "Begin SOAPY_SDR_RX rate test at %f  Msps",
                 device->getSampleRate(SOAPY_SDR_RX, channels.front()) / 1e6);

  auto streamUPtr = std::unique_ptr<SoapySDR::Stream, CStreamDeleter>(
      stream, CStreamDeleter(device));

  mThreadHandle = std::async(std::launch::async,
                             StreamLoop,
                             std::ref(dataQueue),
                             std::move(device),
                             std::move(streamUPtr),
                             direction,
                             channels.size(),
                             elemSize);
}

std::string CDeviceStreamRtl::Impl::StreamLoop(
    data_queue::RawQueue& dataQueue,
    std::shared_ptr<SoapySDR::Device> device,
    std::unique_ptr<SoapySDR::Stream, CStreamDeleter> stream,
    const int direction,
    const size_t numChans,
    const size_t elemSize) {
  LOG_FUNC();

  // allocate buffers for the stream read/write
  const auto numElems = device->getStreamMTU(stream.get());
  std::vector<std::vector<std::int8_t> > buffMem(
      numChans, std::vector<std::int8_t>(elemSize * numElems));
  std::vector<void*> buffs(numChans);
  for (size_t i = 0; i < numChans; i++) {
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

  SoapySDR::logf(SOAPY_SDR_INFO,
                 "Starting stream loop, press Ctrl+C to exit...");
  device->activateStream(stream.get());
  signal(SIGINT, sigIntHandler);
  while (not streamLoopDone) {
    int ret(0);
    int flags(0);
    long long timeNs(0);
    switch (direction) {
      case SOAPY_SDR_RX:
        ret = device->readStream(
            stream.get(), buffs.data(), numElems, flags, timeNs);
        break;
      case SOAPY_SDR_TX:
        ret = device->writeStream(
            stream.get(), buffs.data(), numElems, flags, timeNs);
        break;
    }

    if (SOAPY_SDR_TIMEOUT == ret)
      continue;
    if (SOAPY_SDR_OVERFLOW == ret) {
      overflows++;
      continue;
    }
    if (SOAPY_SDR_UNDERFLOW == ret) {
      underflows++;
      continue;
    }
    if (ret < 0) {
      SoapySDR::logf(SOAPY_SDR_ERROR,
                     "Unexpected stream error %s",
                     SoapySDR::errToStr(ret));
      break;
    }
    totalSamples += ret;

    const auto now = std::chrono::high_resolution_clock::now();
    if (timeLastSpin + std::chrono::milliseconds(300) < now) {
      timeLastSpin = now;
      static const char spin[] = {"|/-\\"};
      printf("\b%c", spin[(spinIndex++) % 4]);
      fflush(stdout);
    }
    // occasionally read out the stream status (non blocking)
    if (timeLastStatus + std::chrono::seconds(1) < now) {
      timeLastStatus = now;
      while (true) {
        size_t chanMask;
        int flags;
        long long timeNs;
        ret =
            device->readStreamStatus(stream.get(), chanMask, flags, timeNs, 0);
        if (SOAPY_SDR_OVERFLOW == ret)
          overflows++;
        else if (SOAPY_SDR_UNDERFLOW == ret)
          underflows++;
        else if (SOAPY_SDR_TIME_ERROR == ret) {
        } else
          break;
      }
    }
    if (timeLastPrint + std::chrono::seconds(5) < now) {
      timeLastPrint = now;
      const auto timePassed =
          std::chrono::duration_cast<std::chrono::microseconds>(now -
                                                                startTime);
      const auto sampleRate = double(totalSamples) / timePassed.count();
      printf(
          "\b%g Msps\t%g MBps", sampleRate, sampleRate * numChans * elemSize);
      if (0u != overflows)
        printf("\tOverflows %u", overflows);
      if (0u != underflows)
        printf("\tUnderflows %u", underflows);
      printf("\n ");
    }

    for (const auto& data : buffMem) {
      dataQueue.Push(data);
    }
  }

  constexpr auto format =
      "Stream: %p %g Msps\t%g MBps\tOverflows "
      "%u\tUnderflows %u TotalSamples %llu";

  const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(
      timeLastPrint - startTime);

  const auto sampleRate = double(totalSamples) / timePassed.count();

  const auto dataSize = snprintf(nullptr,
                                 0u,
                                 format,
                                 stream.get(),
                                 sampleRate,
                                 sampleRate * numChans * elemSize,
                                 overflows,
                                 underflows,
                                 totalSamples);

  std::string report(dataSize + 1, '\0');

  snprintf(report.data(),
           dataSize,
           format,
           stream.get(),
           sampleRate,
           sampleRate * numChans * elemSize,
           overflows,
           underflows,
           totalSamples);

  return report;
}

}  // namespace device_stream
