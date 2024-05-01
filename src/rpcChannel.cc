#include "rpcChannel.h"
#include "rpcController.h"
#include <string>
#include "RpcHeader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "RpcApplication.h"
#include "ZkClient.h"


// 数据传输格式：header_size + service_name method_name args_size + args

// 所有通过stub代理对象调用的RPC方法，统一由 CallMethod 做RPC方法调用的数据序列化和网络发送 
void rpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                                google::protobuf::RpcController* controller, 
                                const google::protobuf::Message* request,
                                google::protobuf::Message* response,
                                google::protobuf:: Closure* done){

    // 通过 method 获取服务名和方法名
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name(); // service_name
    std::string method_name = method->name(); // method_name

    // 获取参数的序列化字符串长度 args_size
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str)){
        args_size = args_str.size();
    }
    else{
        // 通过控制器获取CallMethod函数的具体执行情况
        controller->SetFailed("serialize request error!");
        return;
    }
    
    // 定义RPC请求header
    tinyrpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str)){
        header_size = rpc_header_str.size();
    }
    else{
        controller->SetFailed("serialize RPC header error!");
        return;
    }

    // 组织请求字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4)); // header_size
    send_rpc_str += rpc_header_str; //service_name method_name args_size
    send_rpc_str += args_str; // args

    std::cout << "============================================" << std::endl;
    std::cout << "header_size:  " << header_size << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name:  " << method_name << std::endl; 
    std::cout << "args_str:     " << args_str << std::endl; 
    std::cout << "============================================" << std::endl;

    // 服务调用者不要求高并发，这里使用TCP编程，完成RPC方法的远程调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd){
        char errtxt[512] = {0};
        sprintf(errtxt, "create client socket error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return ;
    }

    // 查询zk中service_name的method_name服务所在的host信息
    ZkClient zkCli;
    zkCli.Start();
    // eg: /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;
    
    std::string host_data = zkCli.GetData(method_path.c_str()); // 格式 ip:port
    if (host_data == ""){
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1){
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }

    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str()); 

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接对应的RPC服务提供节点
    if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr))){
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "server connect error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 发送请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0)){
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 接收请求响应
    char recv_buf[512] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0))){
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 反序列化rpc调用的响应数据

    // 问题：recv_buf 遇到 \0 默认结束，导致反序列化失败
    // std::string response_str(recv_buf, 0, recv_size);
    // if (!response->ParseFromString(response_str))

    if (!response->ParseFromArray(recv_buf, recv_size)){
        close(clientfd);
        char errtxt[1024] = {0};
        sprintf(errtxt, "parse error! response_str:%s", recv_buf);
        controller->SetFailed(errtxt);
        return;
    }

    close(clientfd);
}