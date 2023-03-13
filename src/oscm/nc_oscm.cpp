#include "nc_oscm.h"

#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <iostream>

#include "oscm_server_client.h"

#ifdef __cplusplus
extern "C" {
#endif

OSCMServerClient* oscm_server_client;

void init_oscm_lib() {
    std::cout << "Initialize OSCM library\n"
              << std::flush;
    oscm_server_client = new OSCMServerClient(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
}

void deinit_oscm_lib() {
    std::cout << "Deinitialize OSCM library\n"
              << std::flush;
    free(oscm_server_client);
}

struct oscm_result* get_oscm_metadata(char* key) {
    std::cout << "[get_oscm_metadata] Key: " << key << std::endl;
    struct oscm_result* ret = oscm_server_client->GetMetadata(key);
    return ret;
}

void put_oscm_metadata(char* block_id, char* block_info) {
    std::cout << "[put_oscm_metadata] BlockId: " << block_id << ", BlockInfo: " << block_info << std::endl;
    int status = oscm_server_client->PutMetadata(block_id, block_info);
    assert(status == 1);
}

void delete_oscm_metadata(char* key) {
    std::cout << "[delete_oscm_metadata] Key: " << key << std::endl;
    int status = oscm_server_client->DeleteMetadata(key);
    assert(status == 1);
}

#ifdef __cplusplus
}
#endif