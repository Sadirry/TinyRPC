#include "Logger.h"
#include <time.h>
#include <iostream>

// 获取日志单例
Logger& Logger::GetInstance(){
    static Logger logger;
    return logger;
}

Logger::Logger(){

    // 匿名函数启动专门的写日志线程
    std::thread writeLogTask([&](){
        while(true){
            // 获取当前的日期，以日期命名日志文件
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year+1900, nowtm->tm_mon+1, nowtm->tm_mday);

            FILE *pf = fopen(file_name, "a+");
            if (pf == nullptr){
                std::cout << "logger file : " << file_name << " open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::pair<int, std::string> msg = lockqueue_.Pop(); // 阻塞等待日志信息加入queue中

            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d =>[%s] ", 
                    nowtm->tm_hour, 
                    nowtm->tm_min, 
                    nowtm->tm_sec,
                    (msg.first == INFO ? "INFO" : "ERROR"));
            msg.second.insert(0, time_buf);
            msg.second.append("\n");

            fputs(msg.second.c_str(), pf);
            fclose(pf);
        }
    });
    // 设置线程分离
    writeLogTask.detach();
}

// 设置日志级别 
void Logger::SetLogLevel(LogLevel level){
    loglevel_ = level;
}

// 写日志, 将日志信息写入lockqueue缓冲区中
void Logger::Log(std::pair<int, std::string> msg){
    lockqueue_.Push(msg);
}