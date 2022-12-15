#ifndef __DATA_QUEUE_H__
#define __DATA_QUEUE_H__

#include <algorithm>
#include <memory>
#include <queue>
#include <vector>

namespace data_queue {

template <class DataType = std::vector<std::int8_t>,
          class Queue = std::queue<DataType> >
class CDataQueue {
 public:
  /**
   * @brief ctor
   */
  CDataQueue();

  /**
   * @brief Move ctor
   */
  CDataQueue(CDataQueue&&);

  /**
   * @brief dtor
   */
  ~CDataQueue();

  /**
   * @brief Returns whether the queue is empty
   * @return Whether its size is zero return true, otherwise false
   */
  bool Empty() const;

  /**
   * @brief Returns the number of elements in the queue
   * @return The number of elements in the queue
   */
  size_t Size() const;

  /**
   * @brief Inserts a new element at the end of the queue,
   * after its current last element
   * @param val Value to which the inserted element is initialized.
   * Element to be added to the queue.n
   */
  void Push(const DataType& val);

  /**
   * @brief Removes the next element
   * @param val set reference of the removes element
   */
  bool Pop(DataType& val);

  /**
   * @brief Waits for the new element in the queue
   */
  void WaitDataReady();

  /**
   * @brief Waits for the all data in queue to be processed
   */
  void WaitQueueProcessed();

  /**
   * @brief Indicates if the data send in queue is stopped
   */
  bool IsQueueStopped() const;

  /**
   * @brief Stops sending data to the queue
   */
  void StopQueue();

  /**
   * @brief Removes all data from the queue
   */
  void ResetData();

 private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

using RawQueue = CDataQueue<std::vector<std::int8_t>,
                            std::queue<std::vector<std::int8_t> > >;

}  // namespace data_queue

#endif  // __DATA_QUEUE_H__
