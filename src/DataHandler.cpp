#include "DataHandler.h"

#include <complex>
#include <future>
#include <kfr/base.hpp>
#include <kfr/dft.hpp>
#include <kfr/dsp.hpp>
#include <kfr/io.hpp>

#include "Utility.h"

namespace data_handler {
struct CDataHandler::Impl {
    ~Impl() {
        LOG_FUNC();

        if (mQueueHandle.valid()) {
            mQueueHandle.get();
        }
    }

    void DataHandler();
    data_queue::RawQueue mQueue;
    std::future<void> mQueueHandle;
};

void CDataHandler::Impl::DataHandler() {
    LOG_FUNC();

    while (not mQueue.IsQueueStopped()) {
        mQueue.WaitDataReady();

        std::vector<std::int8_t> data;
        while (mQueue.Pop(data)) {
            SoapySDR::logf(
                SOAPY_SDR_INFO, "Received msg size: %u", data.size());

            // fft size
            const size_t size = data.size() * 0.5;
            std::vector<kfr::complex<kfr::fbase>> complexData(size * 0.5);
            for (size_t i = 0; i < data.size() - 1; i += 2) {
                complexData.push_back(
                    kfr::complex<kfr::fbase>(data[i], data[i + 1]));
            }

            // initialize input & output buffers
            // kfr::univector<kfr::complex<kfr::fbase>, size> in = kfr::sin(
            //     kfr::linspace(0.0, kfr::c_pi<kfr::fbase, 2> * 4.0, size));
            kfr::univector<kfr::complex<kfr::fbase>> in(complexData.begin(),
                                                        complexData.end());

            kfr::univector<kfr::complex<kfr::fbase>> out(size);
            out = kfr::scalar(kfr::qnan);

            // initialize fft
            const kfr::dft_plan<kfr::fbase> dft(size);

            dft.dump();

            SoapySDR::logf(SOAPY_SDR_INFO, "dft.temp_size: %u", dft.temp_size);

            // allocate work buffer for fft (if needed)
            kfr::univector<kfr::u8> temp(dft.temp_size);

            // perform forward fft
            dft.execute(out, in, temp);

            // scale output
            out = out / size;

            // get magnitude and convert to decibels
            kfr::univector<kfr::fbase> dB = kfr::amp_to_dB(kfr::cabs(out));

            kfr::println("max dB: ", kfr::maxof(dB));
            kfr::println("min dB: ", kfr::minof(dB));
            kfr::println("mean dB: ", kfr::mean(dB));
            kfr::println("rms dB: ", kfr::rms(dB));

            // kfr::println(in);
            // kfr::println();
            // kfr::println(dB);
        }
    }
}

CDataHandler::CDataHandler() : mImpl(std::make_unique<CDataHandler::Impl>()) {}
CDataHandler::~CDataHandler() = default;
CDataHandler::CDataHandler(CDataHandler&&) = default;

void CDataHandler::StartHandling() const {
    LOG_FUNC();

    mImpl->mQueueHandle = std::async(
        std::launch::async, &CDataHandler::Impl::DataHandler, mImpl.get());
}

data_queue::RawQueue& CDataHandler::GetQueue() const {
    return mImpl->mQueue;
}

}  // namespace data_handler
