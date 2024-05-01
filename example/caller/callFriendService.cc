#include <iostream>
#include "RpcApplication.h"
#include "friend.pb.h"

int main(int argc, char **argv){
    // 调用框架初始化函数（只初始化一次）
    RpcApplication::Init(argc, argv);

    fixbug::FiendServiceRpc_Stub stub(new rpcChannel());


    // 请求参数
    fixbug::GetFriendsListRequest request;
    request.set_userid(1000);

    // 响应
    fixbug::GetFriendsListResponse response;

    
    // 发起 RPC 方法调用（同步的调用过程）  rpcChannel::callmethod
    // 利用 RpcChannel->RpcChannel::callMethod 集中来实现所有RPC方法调用的参数序列化和网络发送
    // 调用远程发布的GetFriendsList方法
    rpcController controller; // 利用控制器获取调用过程具体信息
    stub.GetFriendsList(&controller, &request, &response, nullptr);

    // 调用完成，读取结果
    if (controller.Failed()){ // 调用失败
        std::cout << controller.ErrorText() << std::endl;
    }
    else{  // 调用成功
        if (0 == response.result().errcode()){
            std::cout << "GetFriendsList response success! " << std::endl;
            int size = response.friends_size();
            for (int i=0; i < size; i++){
                std::cout << "index:" << (i+1) << " name:" << response.friends(i) << std::endl;
            }
        }
        else{
            std::cout << "GetFriendsList response error: " << response.result().errmsg() << std::endl;
        }
    }
    return 0;
}