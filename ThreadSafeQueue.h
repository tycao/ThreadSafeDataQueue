#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

template<typename T>
class ThreadSafeQueue final
{
public:
    using value_type = T;
    //typedef typename std::queue<T>::size_type size_type;
    using size_type = typename std::queue<T>::size_type;

    // explicit ThreadSafeQueue(typename /* ????typename*/ std::queue<T>::size_type queueSizeMax 
    //              = std::numeric_limits<typename /* ????typename*/ std::queue<T>::size_type>::max()):
    //   queueSizeMax_(queueSizeMax) { }  
    explicit ThreadSafeQueue(size_type queueSizeMax = std::numeric_limits<size_type>::max()) :
        queueSizeMax_(queueSizeMax) { }

    ThreadSafeQueue(const ThreadSafeQueue& src) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue& rhs) = delete;

    enum TQueueResult
    {
        qrNoError = 0,//returned successfully.
        qrFull,  // currently full
        qrLocked // currently locked
    };

    size_type getMaxSize() const { return queueSizeMax_; }
    size_type getCurrentSize() const {
        std::lock_guard<std::mutex> scopeLock(mutex_);
        return queue_.size();
    }

    bool isFull() const {
        std::lock_guard<std::mutex> scopeLock(mutex_);
        return queue_.size() == queueSizeMax_;
    }

    bool isEmpty() const {
        std::lock_guard<std::mutex> scopeLock(mutex_);
        return queue_.empty();
    }

    // pData??????????????bool isEmpty();
    // pData???? ??????????
    bool front(T* pData = nullptr)
    {
        std::lock_guard<std::mutex> scopeLock(mutex_);
        const bool boResult = !queue_.empty();
        if (boResult && pData) {
            *pData = queue_.front();
        }
        return boResult;
    }

    TQueueResult push(const T& t);
    TQueueResult push(T&& t);

    bool pop(T* pData = nullptr);

private:
    std::queue<T> queue_;
    // typename /* ????typename*/ std::queue<T>::size_type queueSizeMax_;
    size_type queueSizeMax_;

    std::mutex mutex_;
    std::condition_variable conditionVariable_;
};

template<typename T>
bool ThreadSafeQueue<T>::pop(T* pData)
{
    printf("%s: Enter\n", __FUNCTION__);
    std::unique_lock<std::mutex> scopeLock(mutex_);
    printf("wait for data...\n");
    conditionVariable_.wait(scopeLock, [this] { return !queue_.empty(); });
    printf("extractData...\n");

    if (pData) {
        *pData = std::move(queue_.front());
    }
    queue_.pop();
    printf("%s: Exit\n", __FUNCTION__);
    return true;
}

template<typename T>
typename ThreadSafeQueue<T>::TQueueResult ThreadSafeQueue<T>::push(const T& t)
{
    printf("%s: Enter\n", __FUNCTION__);
    std::lock_guard<std::mutex> scopeLock(mutex_);
    if (queue_.size() >= queueSizeMax_) {
        printf("queue full\n");
        return qrFull;
    }

    queue_.push(t);
    conditionVariable_.notify_one();
    printf("push %d \n", t);
    printf("%s: Exit\n", __FUNCTION__);
    return qrNoError;
}

template<typename T>
typename ThreadSafeQueue<T>::TQueueResult ThreadSafeQueue<T>::push(T&& t)
{
    printf("%s: Enter\n", __FUNCTION__);
    std::lock_guard<std::mutex> scopeLock(mutex_);
    if (queue_.size() >= queueSizeMax_) {
        printf("queue full\n");
        return qrFull;
    }

    queue_.emplace(t);
    conditionVariable_.notify_one();
    printf("push %d \n", t);
    printf("%s: Exit\n", __FUNCTION__);
    return qrNoError;
}