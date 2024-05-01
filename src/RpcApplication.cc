#include "RpcApplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

RpcConfig RpcApplication::config_;

static void ShowArgsHelp(){
    std::cout<<"format: command -i <config json file>" << std::endl;
}

void RpcApplication::Init(int argc, char **argv){
    if (argc < 2){
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1){
        switch (c){
        case 'i':
            config_file = optarg;
            break;
        case '?':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // 加载配置文件
    config_.LoadConfigFile(config_file.c_str());

    // std::cout << "rpcserverip:" << config_.Load("rpcserverip") << std::endl;
    // std::cout << "rpcserverport:" << config_.Load("rpcserverport") << std::endl;
    // std::cout << "zookeeperip:" << config_.Load("zookeeperip") << std::endl;
    // std::cout << "zookeeperport:" << config_.Load("zookeeperport") << std::endl;
}

RpcApplication& RpcApplication::GetInstance(){
    static RpcApplication app;
    return app;
}

RpcConfig& RpcApplication::GetConfig(){
    return config_;
}