#include "DataHandler.h"

#include <future>

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
