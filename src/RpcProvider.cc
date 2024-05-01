#include "RpcProvider.h"
#include "RpcApplication.h"
#include "RpcHeader.pb.h"
#include "Logger.h"
#include "ZkClient.h"

/*
通过 serviceMap_ 中的 service_name 找到 service描述结构体
通过结构体参数 methodMap_ 中的 method_name 找到 method 方法对象
*/
// 提供给外部用户使用，可以发布RPC方法的函数接口
// 简言之填充 serviceMap_ 
void RpcProvider::NotifyService(google::protobuf::Service *service){
    ServiceInfo service_info;

    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    
    // 获取服务名
    std::string service_name = pserviceDesc->name();

    // 获取服务对象 service 的方法数量
    int methodCount = pserviceDesc->method_count();

    LOG_INFO("service_name: %s", service_name.c_str());

    for (int i=0; i < methodCount; ++i){
        // 获取服务对象的服务方法的抽象描述
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name(); // 获取方法名
        service_info.methodMap_.insert({method_name, pmethodDesc});

        LOG_INFO("method_name: %s", method_name.c_str());
    }
    service_info.service_ = service;
    serviceMap_.insert({service_name, service_info});
}

// 启动RPC服务节点，提供RPC远程网络调用服务
void RpcProvider::Run(){
    
    // 读取配置文件rpcserver本机的信息
    std::string ip = RpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(RpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    InetAddress address(port,ip);

    // 创建TcpServer对象
    TcpServer server(&eventLoop_, address, "RpcProvider");

    // 绑定连接回调和读写回调
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, 
            std::placeholders::_2, std::placeholders::_3));

    // 设置Muduo库的子线程数
    server.setThreadNum(4);

    // 将当前RPC节点发布的服务全部注册到zk中，使得客户端可从zk中发现服务
    // zkclient使用网络I/O线程每隔 1/3 * timeout 的时间（10s）向zk服务端发送ping消息（心跳机制）
    ZkClient zkCli;
    zkCli.Start();

    // service_name为永久性节点，method_name为临时性节点
    for (auto &sp : serviceMap_) {
        // /service_name   /UserServiceRpc
        std::string service_path = "/" + sp.first;
        // 先创建父节点才能创建子节点
        zkCli.Create(service_path.c_str(), nullptr, 0);

        for (auto &mp : sp.second.methodMap_){
            // /service_name/method_name   eg: /UserServiceRpc/Login 存储当前RPC服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);

            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    // RPC服务端准备启动，打印信息
    std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;

    // 启动网络服务
    server.start();
    eventLoop_.loop(); 
}

// 连接回调
void RpcProvider::OnConnection(const TcpConnectionPtr &conn){
    if (!conn->connected()){
        conn->shutdown(); // 与client连接断开
    }
}

/*
框架中 RpcProvider 和 RpcConsumer 通信采用 protobuf 数据类型
定义proto的message类型，进行数据头的序列化和反序列化，防止粘包
   
header_size(4个字节) + header_str + args_str
header_str：service_name method_name args_size 
*/

// 读写事件回调
void RpcProvider::OnMessage(const TcpConnectionPtr &conn, 
                            Buffer *buffer, 
                            Timestamp){

    // 从网络上接收RPC调用请求的字符流
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    // 根据 header_size 读取数据头的原始字符流，反序列化数据
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    tinyrpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str)){
        // 数据头反序列化成功：
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else{
        // 数据头反序列化失败：
        std::cout << "rpc_header_str: " << rpc_header_str << " parse error!" << std::endl;
        return;
    }

    // 获取请求参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size:  " << header_size << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name:  " << method_name << std::endl; 
    std::cout << "args_str:     " << args_str << std::endl; 
    std::cout << "============================================" << std::endl;

    // 获取service对象和method对象
    auto it = serviceMap_.find(service_name);
    if (it == serviceMap_.end()){
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.methodMap_.find(method_name);
    if (mit == it->second.methodMap_.end()){
        std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.service_; // 获取service对象
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取method对象

    // 生成RPC方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str)){ // 参数反序列化
        std::cout << "request parse error: " << args_str << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给method重写方法的调用，绑定Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                                    const TcpConnectionPtr&, 
                                                                    google::protobuf::Message*>
                                                                    (this, &RpcProvider::SendRpcResponse, 
                                                                    conn, response);

    // 根据客户端RPC请求，调用当前RPC节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}

// Closure的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const TcpConnectionPtr& conn, google::protobuf::Message *response){
    std::string response_str;
    if (response->SerializeToString(&response_str)){ // response进行序列化
        // 序列化成功后，通过网络将执行结果发送回RPC调用方
        conn->send(response_str);
    }
    else{
        std::cout << "serialize response_str error!" << std::endl; 
    }
    conn->shutdown(); // 模拟http的短链接服务，由RpcProvider主动断开连接
}