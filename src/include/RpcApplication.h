#pragma once

#include "RpcConfig.h"
#include "rpcChannel.h"
#include "rpcController.h"

// rpc框架的基础类，负责框架的初始化操作
class RpcApplication{
public:
    static void Init(int argc, char **argv);
    static RpcApplication& GetInstance();
    static RpcConfig& GetConfig();
private:
    static RpcConfig config_;

    RpcApplication(){}
    RpcApplication(const RpcApplication&) = delete;
    RpcApplication(RpcApplication&&) = delete;
};