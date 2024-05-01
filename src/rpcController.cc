#include "rpcController.h"

rpcController::rpcController(){
    failed_ = false;
    errText_ = "";
}

void rpcController::Reset(){
    failed_ = false;
    errText_ = "";
}

bool rpcController::Failed() const{
    return failed_;
}

std::string rpcController::ErrorText() const{
    return errText_;
}

void rpcController::SetFailed(const std::string& reason){
    failed_ = true;
    errText_ = reason;
}

// 未实现的功能
void rpcController::StartCancel(){}
bool rpcController::IsCanceled() const {return false;}
void rpcController::NotifyOnCancel(google::protobuf::Closure* callback) {}