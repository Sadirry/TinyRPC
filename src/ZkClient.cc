#include "ZkClient.h"
#include "RpcApplication.h"
#include <semaphore.h>
#include <iostream>

// 全局watcher观察器, 用于zkserver给zkclient的通知连接成功
void global_watcher(zhandle_t *zh, int type,
                   int state, const char *path, void *watcherCtx){

    if (type == ZOO_SESSION_EVENT){  // 回调的消息类型是和会话相关的消息类型
		if (state == ZOO_CONNECTED_STATE){  // zkclient和zkserver连接成功
			sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem); // 信号量通知客户端连接成功
		}
	}
}

ZkClient::ZkClient() : zhandle_(nullptr){ }

ZkClient::~ZkClient(){
    if (zhandle_ != nullptr){
        zookeeper_close(zhandle_); // 关闭句柄，释放资源
    }
}

// 连接zkserver
void ZkClient::Start(){
    std::string host = RpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = RpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;
    
	/*
	使用 zookeeper_mt zk的多线程版本
	zookeeper的API客户端程序会提供三个线程：1. API调用线程 2. 网络I/O线程  pthread_create创建线程由 poll 网络通信
		3. watcher回调线程 pthread_create
	*/
    zhandle_ = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0); // 30s超时时间
    // zookeeper_init会立刻返回，由global_watcher通知是否成功连接zkserver

	if (nullptr == zhandle_) {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0); // 信号量初始为0
    zoo_set_context(zhandle_, &sem);

    sem_wait(&sem); // 阻塞直至连接成功
    std::cout << "zookeeper_init success!" << std::endl;
}

// 在zkserver上根据指定的path创建znode节点
void ZkClient::Create(const char *path, const char *data, int datalen, int state){
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
	// 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
	flag = zoo_exists(zhandle_, path, 0, nullptr);

	if (ZNONODE == flag){ // path的znode节点不存在
		// 创建指定path的znode节点了
		flag = zoo_create(zhandle_, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
		if (flag == ZOK){
			std::cout << "znode create success. path: " << path << std::endl;
		}
		else{
			std::cout << "flag: " << flag << std::endl;
			std::cout << "znode create error. path: " << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

// 根据指定的path，获取znode节点的值
std::string ZkClient::GetData(const char *path){
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(zhandle_, path, 0, buffer, &bufferlen, nullptr);
	if (flag != ZOK){
		std::cout << "get znode error. path: " << path << std::endl;
		return "";
	}
	else{
		return buffer;
	}
}