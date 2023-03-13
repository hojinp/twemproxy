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
    std::cout << "[OSCMSeverClient::GetMetaData] Key: " << key << std::endl;
    GetParam getParam;
    GetReturn getReturn;
    std::string key_str = std::string(key);
    getParam.set_key(key_str);

    grpc::ClientContext context;
    grpc::Status status = stub_->GetMetadata(&context, getParam, &getReturn);
    if (!status.ok()) {
        std::cout << "GetFeature rpc failed." << std::endl;
        return NULL;
    }

    oscm_result* ret = (oscm_result*)malloc(sizeof(oscm_result));
    if (!getReturn.exists()) {
        std::cout << "[OSCMSeverClient::GetMetaData] Doesn't exist in OSCMServer" << std::endl;
        ret->exist = 0;
    } else {
        std::cout << "[OSCMSeverClient::GetMetaData] Exist in OSCMServer" << std::endl;
        std::string blockid = getReturn.blockid();
        strncpy(ret->block_id, blockid.c_str(), blockid.length());
        ret->block_id[blockid.length()] = '\0';
        ret->offset = getReturn.offset();
        ret->size = getReturn.size();
        ret->exist = 1;
    }
    return ret;
}

#ifdef __cplusplus
}
#endif