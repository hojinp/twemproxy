// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: oscm_server.proto
#ifndef GRPC_oscm_5fserver_2eproto__INCLUDED
#define GRPC_oscm_5fserver_2eproto__INCLUDED

#include "oscm_server.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_generic_service.h>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/method_handler_impl.h>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <grpcpp/impl/codegen/rpc_method.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/status.h>
#include <grpcpp/impl/codegen/stub_options.h>
#include <grpcpp/impl/codegen/sync_stream.h>

namespace grpc {
class CompletionQueue;
class Channel;
class ServerCompletionQueue;
class ServerContext;
}  // namespace grpc

class MetadataService final {
 public:
  static constexpr char const* service_full_name() {
    return "MetadataService";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status GetMetadata(::grpc::ClientContext* context, const ::GetParam& request, ::GetReturn* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::GetReturn>> AsyncGetMetadata(::grpc::ClientContext* context, const ::GetParam& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::GetReturn>>(AsyncGetMetadataRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::GetReturn>> PrepareAsyncGetMetadata(::grpc::ClientContext* context, const ::GetParam& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::GetReturn>>(PrepareAsyncGetMetadataRaw(context, request, cq));
    }
    virtual ::grpc::Status PutMetadata(::grpc::ClientContext* context, const ::PutParam& request, ::PutReturn* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::PutReturn>> AsyncPutMetadata(::grpc::ClientContext* context, const ::PutParam& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::PutReturn>>(AsyncPutMetadataRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::PutReturn>> PrepareAsyncPutMetadata(::grpc::ClientContext* context, const ::PutParam& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::PutReturn>>(PrepareAsyncPutMetadataRaw(context, request, cq));
    }
    virtual ::grpc::Status DeleteMetadata(::grpc::ClientContext* context, const ::DeleteParam& request, ::DeleteReturn* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::DeleteReturn>> AsyncDeleteMetadata(::grpc::ClientContext* context, const ::DeleteParam& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::DeleteReturn>>(AsyncDeleteMetadataRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::DeleteReturn>> PrepareAsyncDeleteMetadata(::grpc::ClientContext* context, const ::DeleteParam& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::DeleteReturn>>(PrepareAsyncDeleteMetadataRaw(context, request, cq));
    }
    class experimental_async_interface {
     public:
      virtual ~experimental_async_interface() {}
      virtual void GetMetadata(::grpc::ClientContext* context, const ::GetParam* request, ::GetReturn* response, std::function<void(::grpc::Status)>) = 0;
      virtual void PutMetadata(::grpc::ClientContext* context, const ::PutParam* request, ::PutReturn* response, std::function<void(::grpc::Status)>) = 0;
      virtual void DeleteMetadata(::grpc::ClientContext* context, const ::DeleteParam* request, ::DeleteReturn* response, std::function<void(::grpc::Status)>) = 0;
    };
    virtual class experimental_async_interface* experimental_async() { return nullptr; }
  private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::GetReturn>* AsyncGetMetadataRaw(::grpc::ClientContext* context, const ::GetParam& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::GetReturn>* PrepareAsyncGetMetadataRaw(::grpc::ClientContext* context, const ::GetParam& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::PutReturn>* AsyncPutMetadataRaw(::grpc::ClientContext* context, const ::PutParam& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::PutReturn>* PrepareAsyncPutMetadataRaw(::grpc::ClientContext* context, const ::PutParam& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::DeleteReturn>* AsyncDeleteMetadataRaw(::grpc::ClientContext* context, const ::DeleteParam& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::DeleteReturn>* PrepareAsyncDeleteMetadataRaw(::grpc::ClientContext* context, const ::DeleteParam& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel);
    ::grpc::Status GetMetadata(::grpc::ClientContext* context, const ::GetParam& request, ::GetReturn* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::GetReturn>> AsyncGetMetadata(::grpc::ClientContext* context, const ::GetParam& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::GetReturn>>(AsyncGetMetadataRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::GetReturn>> PrepareAsyncGetMetadata(::grpc::ClientContext* context, const ::GetParam& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::GetReturn>>(PrepareAsyncGetMetadataRaw(context, request, cq));
    }
    ::grpc::Status PutMetadata(::grpc::ClientContext* context, const ::PutParam& request, ::PutReturn* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::PutReturn>> AsyncPutMetadata(::grpc::ClientContext* context, const ::PutParam& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::PutReturn>>(AsyncPutMetadataRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::PutReturn>> PrepareAsyncPutMetadata(::grpc::ClientContext* context, const ::PutParam& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::PutReturn>>(PrepareAsyncPutMetadataRaw(context, request, cq));
    }
    ::grpc::Status DeleteMetadata(::grpc::ClientContext* context, const ::DeleteParam& request, ::DeleteReturn* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::DeleteReturn>> AsyncDeleteMetadata(::grpc::ClientContext* context, const ::DeleteParam& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::DeleteReturn>>(AsyncDeleteMetadataRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::DeleteReturn>> PrepareAsyncDeleteMetadata(::grpc::ClientContext* context, const ::DeleteParam& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::DeleteReturn>>(PrepareAsyncDeleteMetadataRaw(context, request, cq));
    }
    class experimental_async final :
      public StubInterface::experimental_async_interface {
     public:
      void GetMetadata(::grpc::ClientContext* context, const ::GetParam* request, ::GetReturn* response, std::function<void(::grpc::Status)>) override;
      void PutMetadata(::grpc::ClientContext* context, const ::PutParam* request, ::PutReturn* response, std::function<void(::grpc::Status)>) override;
      void DeleteMetadata(::grpc::ClientContext* context, const ::DeleteParam* request, ::DeleteReturn* response, std::function<void(::grpc::Status)>) override;
     private:
      friend class Stub;
      explicit experimental_async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class experimental_async_interface* experimental_async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class experimental_async async_stub_{this};
    ::grpc::ClientAsyncResponseReader< ::GetReturn>* AsyncGetMetadataRaw(::grpc::ClientContext* context, const ::GetParam& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::GetReturn>* PrepareAsyncGetMetadataRaw(::grpc::ClientContext* context, const ::GetParam& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::PutReturn>* AsyncPutMetadataRaw(::grpc::ClientContext* context, const ::PutParam& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::PutReturn>* PrepareAsyncPutMetadataRaw(::grpc::ClientContext* context, const ::PutParam& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::DeleteReturn>* AsyncDeleteMetadataRaw(::grpc::ClientContext* context, const ::DeleteParam& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::DeleteReturn>* PrepareAsyncDeleteMetadataRaw(::grpc::ClientContext* context, const ::DeleteParam& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_GetMetadata_;
    const ::grpc::internal::RpcMethod rpcmethod_PutMetadata_;
    const ::grpc::internal::RpcMethod rpcmethod_DeleteMetadata_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status GetMetadata(::grpc::ServerContext* context, const ::GetParam* request, ::GetReturn* response);
    virtual ::grpc::Status PutMetadata(::grpc::ServerContext* context, const ::PutParam* request, ::PutReturn* response);
    virtual ::grpc::Status DeleteMetadata(::grpc::ServerContext* context, const ::DeleteParam* request, ::DeleteReturn* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_GetMetadata : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_GetMetadata() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_GetMetadata() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetMetadata(::grpc::ServerContext* context, const ::GetParam* request, ::GetReturn* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestGetMetadata(::grpc::ServerContext* context, ::GetParam* request, ::grpc::ServerAsyncResponseWriter< ::GetReturn>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_PutMetadata : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_PutMetadata() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_PutMetadata() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status PutMetadata(::grpc::ServerContext* context, const ::PutParam* request, ::PutReturn* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestPutMetadata(::grpc::ServerContext* context, ::PutParam* request, ::grpc::ServerAsyncResponseWriter< ::PutReturn>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_DeleteMetadata : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_DeleteMetadata() {
      ::grpc::Service::MarkMethodAsync(2);
    }
    ~WithAsyncMethod_DeleteMetadata() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status DeleteMetadata(::grpc::ServerContext* context, const ::DeleteParam* request, ::DeleteReturn* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestDeleteMetadata(::grpc::ServerContext* context, ::DeleteParam* request, ::grpc::ServerAsyncResponseWriter< ::DeleteReturn>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(2, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_GetMetadata<WithAsyncMethod_PutMetadata<WithAsyncMethod_DeleteMetadata<Service > > > AsyncService;
  template <class BaseClass>
  class WithGenericMethod_GetMetadata : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_GetMetadata() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_GetMetadata() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetMetadata(::grpc::ServerContext* context, const ::GetParam* request, ::GetReturn* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_PutMetadata : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_PutMetadata() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_PutMetadata() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status PutMetadata(::grpc::ServerContext* context, const ::PutParam* request, ::PutReturn* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_DeleteMetadata : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_DeleteMetadata() {
      ::grpc::Service::MarkMethodGeneric(2);
    }
    ~WithGenericMethod_DeleteMetadata() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status DeleteMetadata(::grpc::ServerContext* context, const ::DeleteParam* request, ::DeleteReturn* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_GetMetadata : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithRawMethod_GetMetadata() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_GetMetadata() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetMetadata(::grpc::ServerContext* context, const ::GetParam* request, ::GetReturn* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestGetMetadata(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_PutMetadata : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithRawMethod_PutMetadata() {
      ::grpc::Service::MarkMethodRaw(1);
    }
    ~WithRawMethod_PutMetadata() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status PutMetadata(::grpc::ServerContext* context, const ::PutParam* request, ::PutReturn* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestPutMetadata(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_DeleteMetadata : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithRawMethod_DeleteMetadata() {
      ::grpc::Service::MarkMethodRaw(2);
    }
    ~WithRawMethod_DeleteMetadata() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status DeleteMetadata(::grpc::ServerContext* context, const ::DeleteParam* request, ::DeleteReturn* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestDeleteMetadata(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(2, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_GetMetadata : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithStreamedUnaryMethod_GetMetadata() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler< ::GetParam, ::GetReturn>(std::bind(&WithStreamedUnaryMethod_GetMetadata<BaseClass>::StreamedGetMetadata, this, std::placeholders::_1, std::placeholders::_2)));
    }
    ~WithStreamedUnaryMethod_GetMetadata() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status GetMetadata(::grpc::ServerContext* context, const ::GetParam* request, ::GetReturn* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedGetMetadata(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::GetParam,::GetReturn>* server_unary_streamer) = 0;
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_PutMetadata : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithStreamedUnaryMethod_PutMetadata() {
      ::grpc::Service::MarkMethodStreamed(1,
        new ::grpc::internal::StreamedUnaryHandler< ::PutParam, ::PutReturn>(std::bind(&WithStreamedUnaryMethod_PutMetadata<BaseClass>::StreamedPutMetadata, this, std::placeholders::_1, std::placeholders::_2)));
    }
    ~WithStreamedUnaryMethod_PutMetadata() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status PutMetadata(::grpc::ServerContext* context, const ::PutParam* request, ::PutReturn* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedPutMetadata(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::PutParam,::PutReturn>* server_unary_streamer) = 0;
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_DeleteMetadata : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithStreamedUnaryMethod_DeleteMetadata() {
      ::grpc::Service::MarkMethodStreamed(2,
        new ::grpc::internal::StreamedUnaryHandler< ::DeleteParam, ::DeleteReturn>(std::bind(&WithStreamedUnaryMethod_DeleteMetadata<BaseClass>::StreamedDeleteMetadata, this, std::placeholders::_1, std::placeholders::_2)));
    }
    ~WithStreamedUnaryMethod_DeleteMetadata() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status DeleteMetadata(::grpc::ServerContext* context, const ::DeleteParam* request, ::DeleteReturn* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedDeleteMetadata(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::DeleteParam,::DeleteReturn>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_GetMetadata<WithStreamedUnaryMethod_PutMetadata<WithStreamedUnaryMethod_DeleteMetadata<Service > > > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_GetMetadata<WithStreamedUnaryMethod_PutMetadata<WithStreamedUnaryMethod_DeleteMetadata<Service > > > StreamedService;
};


#endif  // GRPC_oscm_5fserver_2eproto__INCLUDED
