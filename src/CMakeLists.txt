set(SRC_LIST 
    RpcApplication.cc 
    RpcConfig.cc 
    RpcHeader.pb.cc 
    RpcProvider.cc 
    rpcChannel.cc
    rpcController.cc
    Logger.cc
    ZkClient.cc)

add_library(TinyRPC SHARED ${SRC_LIST})
target_link_libraries(TinyRPC Muduo protobuf pthread)
target_link_libraries(TinyRPC  "/usr/local/lib/libzookeeper_mt.a") # 指定链接到静态库
