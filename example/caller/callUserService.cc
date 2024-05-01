#include <iostream>
#include "RpcApplication.h"
#include "user.pb.h"
#include "rpcChannel.h"

int main(int argc, char **argv){
    // 调用框架初始化函数（只初始化一次）
    RpcApplication::Init(argc, argv);

    fixbug::UserServiceRpc_Stub stub(new rpcChannel());
    
    // 请求参数
    fixbug::LoginRequest request;
    request.set_name("zhangsan");
    request.set_pwd("123456");

    // 响应
    fixbug::LoginResponse response;

    // 发起 RPC 方法调用（同步的调用过程）  rpcChannel::callmethod
    // 利用 RpcChannel->RpcChannel::callMethod 集中来实现所有RPC方法调用的参数序列化和网络发送
    // 调用远程发布的Login方法
    stub.Login(nullptr, &request, &response, nullptr);

    // 注意：这里默认调用成功，没有使用控制器获取调用过程的具体信息，RpcController的使用参考callFriendService.cc

    // 调用完成，读取结果
    if (0 == response.result().errcode()){
        std::cout << "login response success: " << response.sucess() << std::endl;
    }
    else{
        std::cout << "login response error: " << response.result().errmsg() << std::endl;
    }

    // 调用远程发布的Register方法
    fixbug::RegisterRequest req;
    req.set_name("lisi");
    req.set_pwd("666666");
    fixbug::RegisterResponse rsp;

    // 同步的方式发起调用请求，等待返回结果
    stub.Register(nullptr, &req, &rsp, nullptr); 

    // 调用完成，读取结果
    if (0 == rsp.result().errcode()){
        std::cout << "register response success: " << rsp.sucess() << std::endl;
    }
    else{
        std::cout << "register response error: " << rsp.result().errmsg() << std::endl;
    }
    return 0;
}