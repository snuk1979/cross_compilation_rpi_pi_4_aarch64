#include "DataQueue.h"

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "Utility.h"

namespace data_queue {
template <class DataType, class Queue>
struct CDataQueue<DataType, Queue>::Impl {
    Queue mQueue;
    std::mutex mDataGuard;
    std::condition_variable mDataCV;
    volatile std::atomic_bool mIsStopped{false};
};

template <typename DataType, class Queue>
CDataQueue<DataType, Queue>::CDataQueue()
    : mImpl(std::make_unique<CDataQueue<DataType, Queue>::Impl>()) {}

template <typename DataType, class Queue>
CDataQueue<DataType, Queue>::CDataQueue(CDataQueue&&) = default;

template <typename DataType, class Queue>
CDataQueue<DataType, Queue>::~CDataQueue() {
    LOG_FUNC();

    StopQueue();
}

template <typename DataType, class Queue>
bool CDataQueue<DataType, Queue>::Empty() const {
    std::lock_guard lock(mImpl->mDataGuard);
    return mImpl->mQueue.empty();
}

template <typename DataType, class Queue>
size_t CDataQueue<DataType, Queue>::Size() const {
    std::lock_guard lock(mImpl->mDataGuard);
    return mImpl->mQueue.size();
}

template <typename DataType, class Queue>
void CDataQueue<DataType, Queue>::Push(const DataType& val) {
    LOG_FUNC();

    std::lock_guard lock(mImpl->mDataGuard);

    if (mImpl->mIsStopped) {
        return;
    }

    mImpl->mQueue.push(val);
    mImpl->mDataCV.notify_all();
}

template <typename DataType, class Queue>
bool CDataQueue<DataType, Queue>::Pop(DataType& val) {
    LOG_FUNC();

    std::lock_guard lock(mImpl->mDataGuard);

    if (mImpl->mQueue.empty()) {
        return false;
    }

    val = mImpl->mQueue.front();
    mImpl->mQueue.pop();
    mImpl->mDataCV.notify_one();

    return true;
}

template <typename DataType, class Queue>
void CDataQueue<DataType, Queue>::WaitDataReady() {
    LOG_FUNC();

    std::unique_lock lock(mImpl->mDataGuard);

    if (not mImpl->mIsStopped) {
        mImpl->mDataCV.wait(lock, [impl = mImpl.get()]() {
            return (not impl->mQueue.empty() || impl->mIsStopped);
        });
    }
}

template <typename DataType, class Queue>
void CDataQueue<DataType, Queue>::WaitQueueProcessed() {
    LOG_FUNC();

    std::unique_lock lock(mImpl->mDataGuard);

    if (not mImpl->mIsStopped) {
        mImpl->mDataCV.wait(lock, [impl = mImpl.get()]() {
            return (impl->mQueue.empty() || impl->mIsStopped);
        });
    }
}

template <typename DataType, class Queue>
bool CDataQueue<DataType, Queue>::IsQueueStopped() const {
    return mImpl->mIsStopped;
}

template <typename DataType, class Queue>
void CDataQueue<DataType, Queue>::StopQueue() {
    LOG_FUNC();

    if (mImpl->mIsStopped) {
        return;
    }

    mImpl->mIsStopped = true;

    std::lock_guard lock(mImpl->mDataGuard);

    if (not mImpl->mQueue.empty()) {
        Queue queue;
        mImpl->mQueue.swap(queue);
    }

    mImpl->mDataCV.notify_all();
}

template <typename DataType, class Queue>
void CDataQueue<DataType, Queue>::ResetData() {
    LOG_FUNC();

    mImpl->mIsStopped = false;

    std::lock_guard lock(mImpl->mDataGuard);

    if (not mImpl->mQueue.empty()) {
        Queue queue;
        mImpl->mQueue.swap(queue);
    }
}

template class CDataQueue<std::vector<std::int8_t>,
                          std::queue<std::vector<std::int8_t>>>;

}  // namespace data_queue
