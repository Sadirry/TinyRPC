#include <iostream>
#include <string>
#include "friend.pb.h"
#include "RpcApplication.h"
#include "RpcProvider.h"
#include <vector>
#include "Logger.h"


// 服务 FriendService 提供一个方法：GetFriendsList
class FriendService : public fixbug::FiendServiceRpc{
public:
    std::vector<std::string> GetFriendsList(uint32_t userid){
        std::cout << "Doing FriendService: GetFriendsList userid: " << userid << std::endl;
        std::vector<std::string> vec;
        vec.push_back("zhangsan");
        vec.push_back("lisi");
        vec.push_back("wangwu");
        return vec;
    }

    // 重写基类方法
    void GetFriendsList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendsListRequest* request,
                       ::fixbug::GetFriendsListResponse* response,
                       ::google::protobuf::Closure* done){
        uint32_t userid = request->userid();

        // 调用本地方法
        std::vector<std::string> friendsList = GetFriendsList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("get friends success!");

        for (std::string &name : friendsList){
            std::string *p = response->add_friends();
            *p = name;
        }
        done->Run();
    }
};

int main(int argc, char **argv){

    RpcApplication::Init(argc, argv); // 框架的初始化,读取配置文件

    // 定义provider RPC网络服务对象
    RpcProvider provider;

    // 将 UserService 服务发布到 provider 中
    provider.NotifyService(new FriendService());

    // 启动RPC服务发布节点，并将节点发布到zk中
    // 启动后，进程进入阻塞状态，等待远程RPC调用请求
    provider.Run();

    return 0;
}