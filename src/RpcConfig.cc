#include "RpcConfig.h"
#include <iostream>
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

// 负责解析加载配置文件
void RpcConfig::LoadConfigFile(const char *config_file){

    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cout << config_file << " is note exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 解析JSON
    json configJson;
    file >> configJson;

    // 存储到unordered_map中
    ConfigMap["rpcserverip"] = configJson["rpc_server"]["ip"];
    ConfigMap["rpcserverport"] = configJson["rpc_server"]["port"];
    ConfigMap["zookeeperip"] = configJson["zookeeper"]["ip"];
    ConfigMap["zookeeperport"] = configJson["zookeeper"]["port"];
    
}


// 查询配置项信息
std::string RpcConfig::Load(const std::string &key){
    auto it = ConfigMap.find(key);
    if (it == ConfigMap.end()){
        return "";
    }
    return it->second;
}