#ifndef _OSCM_SERVER_CLIENT_H_
#define _OSCM_SERVER_CLIENT_H_

#include "../protobuf/oscm_server.grpc.pb.h"
#include "../protobuf/oscm_server.pb.h"
#include "nc_oscm.h"

#ifdef __cplusplus
extern "C" {
#endif

class OSCMServerClient {
   private:
    std::unique_ptr<MetadataService::Stub> stub_;

   public:
    OSCMServerClient(std::shared_ptr<grpc::Channel> channel);

    oscm_result* GetMetadata(char* key);

    int PutMetadata(char* block_id, char* block_info);

    int DeleteMetadata(char* key);
};

#ifdef __cplusplus
}
#endif

#endif /* _OSCM_SERVER_CLIENT_H_ */