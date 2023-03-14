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

void oscm_init_lib() {
    std::cout << "Initialize OSCM library\n"
              << std::flush;
    oscm_server_client = new OSCMServerClient(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
}

void oscm_deinit_lib() {
    std::cout << "Deinitialize OSCM library\n"
              << std::flush;
    free(oscm_server_client);
}

struct oscm_result* oscm_get_metadata(char* key) {
    std::cout << "[oscm_get_metadata] Key: " << key << std::endl;
    struct oscm_result* ret = oscm_server_client->GetMetadata(key);
    return ret;
}

void oscm_put_metadata(char* block_id, char* block_info) {
    std::cout << "[oscm_put_metadata] BlockId: " << block_id << ", BlockInfo: " << block_info << "\n"
              << std::flush;
    int status = oscm_server_client->PutMetadata(block_id, block_info);
    assert(status == 1);
}

void oscm_delete_metadata(char* key) {
    std::cout << "[oscm_delete_metadata] Key: " << key << std::endl;
    int status = oscm_server_client->DeleteMetadata(key);
    assert(status == 1);
}

#ifdef __cplusplus
}
#endif