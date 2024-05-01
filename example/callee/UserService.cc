#include <iostream>
#include <string>
#include "user.pb.h"
#include "RpcApplication.h"
#include "RpcProvider.h"


// 服务 UserService 提供两个方法：Login 和 Register
class UserService : public fixbug::UserServiceRpc{  // rpc服务提供者继承ServiceRpc类
public:
    bool Login(std::string name, std::string pwd){
        std::cout << "Doing UserService: Login" << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;  
        return true;
    }

    bool Register(std::string name, std::string pwd){
        std::cout << "doing UserService: Register" << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }


    // 重写基类UserServiceRpc的虚函数 重写方法由RPC框架直接调用
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done){

        // 框架给业务上报请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 本地业务
        bool login_result = Login(name, pwd); 

        // 写响应  包括错误码、错误消息、返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("success login");
        response->set_sucess(login_result);

        // 执行回调操作(由框架实现done->run函数，执行对象数据的序列化和网络发送）
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done){
    
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool register_result = Register(name, pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("success register");
        response->set_sucess(register_result);

        done->Run();
    }
};

int main(int argc, char **argv){
    
    RpcApplication::Init(argc, argv); // 框架的初始化,读取配置文件

    // 定义provider RPC网络服务对象
    RpcProvider provider;

    // 将 UserService 服务发布到 provider 中
    provider.NotifyService(new UserService()); 

    // 启动RPC服务发布节点，并将节点发布到zk中
    // 启动后，进程进入阻塞状态，等待远程RPC调用请求
    provider.Run();

    return 0;
}