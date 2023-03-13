#include "oscm_server_client.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <string.h>

#include <iostream>

#include "../protobuf/oscm_server.grpc.pb.h"
#include "../protobuf/oscm_server.pb.h"
#include "nc_oscm.h"

#ifdef __cplusplus
extern "C" {
#endif

OSCMServerClient::OSCMServerClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(MetadataService::NewStub(channel)) {
}

oscm_result* OSCMServerClient::GetMetadata(char* key) {
    std::cout << "[OSCMSeverClient::GetMetadata] Key: " << key << std::endl;
    GetParam get_param;
    GetReturn get_return;
    std::string key_str = std::string(key);
    get_param.set_key(key_str);

    grpc::ClientContext context;
    grpc::Status status = stub_->GetMetadata(&context, get_param, &get_return);
    if (!status.ok()) {
        std::cout << "GetMetadata rpc failed." << std::endl;
        return NULL;
    }

    oscm_result* ret = (oscm_result*)malloc(sizeof(oscm_result));
    if (!get_return.exists()) {
        std::cout << "[OSCMSeverClient::GetMetadata] Doesn't exist in OSCMServer" << std::endl;
        ret->exist = 0;
    } else {
        std::cout << "[OSCMSeverClient::GetMetadata] Exist in OSCMServer" << std::endl;
        std::string blockid = get_return.blockid();
        strncpy(ret->block_id, blockid.c_str(), blockid.length());
        ret->block_id[blockid.length()] = '\0';
        ret->offset = get_return.offset();
        ret->size = get_return.size();
        ret->exist = 1;
    }
    return ret;
}

int OSCMServerClient::PutMetadata(char* block_id, char* block_info) {
    std::cout << "[OSCMServerClient::PutMetadata] BlockId: " << block_id << std::endl;
    PutParam put_param;
    PutReturn put_return;
    std::string block_id_str = std::string(block_id);
    std::string block_info_str = std::string(block_info);
    put_param.set_blockid(block_id_str);
    put_param.set_blockinfo(block_info_str);

    grpc::ClientContext context;
    grpc::Status status = stub_->PutMetadata(&context, put_param, &put_return);
    if (!status.ok()) {
        std::cout << "PutMetadata rpc failed." << std::endl;
        return -1;
    }
    return 1;
}

int OSCMServerClient::DeleteMetadata(char* key) {
    std::cout << "[OSCMServerClient::DeleteMetadata] Key: " << key << std::endl;
    DeleteParam del_param;
    DeleteReturn del_return;
    std::string key_str = std::string(key);
    del_param.set_key(key_str);

    grpc::ClientContext context;
    grpc::Status status = stub_->DeleteMetadata(&context, del_param, &del_return);
    if (!status.ok()) {
        std::cout << "DeleteMetadata rpc failed." << std::endl;
        return -1;
    }
    return 1;
}

#ifdef __cplusplus
}
#endif