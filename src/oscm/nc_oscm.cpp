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

void set_oscm_metadata(char* key, char* block_id, int offset, int size) {
}

#ifdef __cplusplus
}
#endif