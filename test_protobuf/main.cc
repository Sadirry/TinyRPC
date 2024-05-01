#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;

void test1(){
    GetFriendListsResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0); // 成功

    User *user1 = rsp.add_friend_list();
    user1->set_name("zhangsan");
    user1->set_age(18);
    user1->set_sex(User::MAN);

    User *user2 = rsp.add_friend_list();
    user2->set_name("lisi");
    user2->set_age(20);
    user2->set_sex(User::MAN);

    std::cout << rsp.friend_list_size() << std::endl;
    
    std::string send_str;
    rsp.SerializeToString(&send_str); // 序列化

    GetFriendListsResponse rsp_;
    if (rsp_.ParseFromString(send_str)){ // 反序列化
        for(int i=0;i<rsp_.friend_list_size();i++){
            std::cout << rsp_.friend_list(i).name() <<" "<<rsp_.friend_list(i).age()<< std::endl;
        }
    }

}

void test2(){

    LoginRequest req;
    req.set_name("zhangsan");
    req.set_pwd("123456");

    std::string send_str;

    if (req.SerializeToString(&send_str)){ //序列化 
        std::cout << send_str.c_str() << std::endl;
    }

    // 从send_str反序列化一个login请求对象
    LoginRequest req_;
    if (req_.ParseFromString(send_str)){ // 反序列化
        std::cout << req_.name() <<" "<<req_.pwd() << std::endl;
    }
}

int main(){
    test1();
    test2();
    return 0;
}