#ifndef __DATA_HANDLER_H__
#define __DATA_HANDLER_H__

#include <memory>

#include "DataQueue.h"

namespace data_handler {
class CDataHandler {
   public:
    CDataHandler();
    CDataHandler(CDataHandler&&);
    ~CDataHandler();

    void StartHandling() const;
    data_queue::RawQueue& GetQueue() const;

   private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

}  // namespace data_handler

#endif  // __DATA_HANDLER_H__