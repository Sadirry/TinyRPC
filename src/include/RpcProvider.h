#pragma once
#include "google/protobuf/service.h"
#include <Muduo/TcpServer.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

// 专门用于发布RPC服务的网络对象类
class RpcProvider{
public:
    // 提供给外部用户使用，可以发布RPC方法的函数接口
    // 注意为了框架的适用性，函数参数为基类Server
    void NotifyService(google::protobuf::Service *service);

    // 启动RPC服务节点，提供RPC远程网络调用服务
    void Run();

private:

    EventLoop eventLoop_;

    // 定义service服务类型信息
    struct ServiceInfo{
        google::protobuf::Service *service_; // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> methodMap_; // 保存服务方法
    };

    // 存储注册成功的服务对象和其服务方法
    // serviceMap_的key为服务名
    std::unordered_map<std::string, ServiceInfo> serviceMap_;

    // 连接回调
    void OnConnection(const TcpConnectionPtr&);
    // 读写事件回调
    void OnMessage(const TcpConnectionPtr&, Buffer*, Timestamp);
    // Closure回调，序列化RPC的响应并网络发送
    void SendRpcResponse(const TcpConnectionPtr&, google::protobuf::Message*);
};