# set(SRC_LIST UserService.cc ../user.pb.cc)
set(SRC_LIST FriendService.cc ../friend.pb.cc)

add_executable(provider ${SRC_LIST})
target_link_libraries(provider TinyRPC protobuf)