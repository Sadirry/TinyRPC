#pragma once
#include "LockQueue.h"
#include <string>
#include <utility>

// 定义日志级别
enum LogLevel{
    INFO,  // 普通信息
    ERROR, // 错误信息
};


// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        std::string str_c(c); \
        logger.Log(std::make_pair(INFO,c)); \
    } while(0) \


#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        std::string str_c(c); \
        logger.Log(std::make_pair(ERROR,c)); \
    } while(0) \


// TinyRPC框架提供的日志系统
class Logger{
public:
    // 获取日志的单例
    static Logger& GetInstance();

    // 设置日志级别 
    void SetLogLevel(LogLevel level);

    // 写日志
    void Log(std::pair<int, std::string> msg);

private:
    int loglevel_; // 记录日志级别

    // 日志缓冲队列
    LockQueue<std::pair<int, std::string>>  lockqueue_;

    Logger();

    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
};