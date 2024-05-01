#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

// EventLoop: ChannelList Poller
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false){
}

Channel::~Channel(){
}

// Channel 绑定对应的 TcpConnection 新连接 
void Channel::tie(const std::shared_ptr<void> &obj){
    tie_ = obj;
    tied_ = true;
}

// 通过channel所属的EventLoop，调用poller的相应方法，注册fd的events事件
void Channel::update(){
    loop_->updateChannel(this);
}

// 在channel所属的EventLoop中，将当前的channel删掉
void Channel::remove(){
    loop_->removeChannel(this);
}

// fd得到poller通知以后，处理事件的
void Channel::handleEvent(Timestamp receiveTime){
    if (tied_){
        std::shared_ptr<void> guard = tie_.lock();
        if (guard){
            handleEventWithGuard(receiveTime);
        }
    }
    else{
        handleEventWithGuard(receiveTime);
    }
}

// 根据poller通知的channel发生的具体事件，由channel负责调用具体的回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime){
    LOG_INFO("channel handleEvent revents:%d\n", revents_);

    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)){
        if (closeCallback_){
            closeCallback_();
        }
    }

    if (revents_ & EPOLLERR){
        if (errorCallback_){
            errorCallback_();
        }
    }

    if (revents_ & (EPOLLIN | EPOLLPRI)){
        if (readCallback_){
            readCallback_(receiveTime);
        }
    }

    if (revents_ & EPOLLOUT){
        if (writeCallback_){
            writeCallback_();
        }
    }
}