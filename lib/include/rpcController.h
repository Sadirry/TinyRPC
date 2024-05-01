#pragma once
#include <google/protobuf/service.h>
#include <string>

class rpcController : public google::protobuf::RpcController{
public:
    rpcController();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);

    // 未实现的功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);

private:
    bool failed_; // RPC方法执行过程中的状态
    std::string errText_; // RPC方法执行过程中的错误信息
};