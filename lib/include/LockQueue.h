#pragma once
#include <queue>
#include <thread>
#include <mutex>  // 底层为 pthread_mutex_t
#include <condition_variable>  // 底层为 pthread_condition_t

// 异步写日志的日志队列
template<typename T>
class LockQueue{
public:
    // 多个worker线程都会写日志queue 
    void Push(const T &data){
        std::lock_guard<std::mutex> lock(mutex_); // 加锁
        queue_.push(data);
        condvariable_.notify_one(); // 条件变量满足，唤醒写日志线程
    }

    // 由写日志线程调用：读日志queue，写日志文件
    T Pop(){
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty()){ // 日志队列为空，线程进入阻塞 wait 状态
            condvariable_.wait(lock);
        }
        T data = queue_.front();
        queue_.pop();
        return data;
    }
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable condvariable_;
};