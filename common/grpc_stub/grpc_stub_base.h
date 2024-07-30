/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-02-01 11:11:55
 */

#ifndef NAS_COMMON_GRPC_STUB_GRPC_STUB_BASE_H_
#define NAS_COMMON_GRPC_STUB_GRPC_STUB_BASE_H_

#include <map>
#include <string>

#include "base/callback.h"
#include "base/json/json_reader.h"
#include "base/logging.h"

#include "google/protobuf/message.h"
#include "google/protobuf/message_lite.h"
#include "google/protobuf/util/json_util.h"
#include "grpcpp/client_context.h"

#include "grpc_async_call_base.h"
#include "grpc_client_context.hpp"
#include "grpc_hold_state_channel.h"
#include "request_handler_id.hpp"

namespace nas {

using AsyncGrpcResponseHandler =
    base::OnceCallback<void(HandlerRequestId,
                            grpc::Status,
                            std::shared_ptr<nas::GrpcClientContext>,
                            const std::string&)>;
template <typename ResponseType>
using AsyncGrpcRawResponseHandler =
    base::OnceCallback<void(HandlerRequestId,
                            grpc::Status,
                            std::shared_ptr<nas::GrpcClientContext>,
                            const ResponseType&)>;
using GetStubCallbackCallback = base::OnceCallback<void(void)>;
using GetDestServiceIpPortCallback =
    base::OnceCallback<void(const std::string& ip_port)>;
// template <typename ResponseType>
// class AsyncClientCall : public AsyncClientCallBase {
//  public:
//   AsyncClientCall(AsyncGrpcResponseHandler grpc_response_handler,
//                   HandlerRequestId handler_id,
//                   std::shared_ptr<GrpcClientContext> context)
//       : AsyncClientCallBase(context),
//         grpc_response_handler_(std::move(grpc_response_handler)),
//         hander_request_id_(handler_id) {}

//   virtual void HandleResponse() override {
//     std::string grpc_response;
//     if (status.ok()) {
//       google::protobuf::util::MessageToJsonString(response_, &grpc_response);
//     } else {
//       LOG(ERROR) << "AsyncClientCall HandleResponse
//       failed,hander_request_id_:"
//                   << static_cast<int>(hander_request_id_);
//     }
//     std::move(grpc_response_handler_).Run(hander_request_id_, status,
//     context_, grpc_response);
//   }
//  public:
//   ResponseType response_;
//   std::unique_ptr<grpc::ClientAsyncResponseReader<ResponseType>>
//       response_reader;

//  private:
//   AsyncGrpcResponseHandler grpc_response_handler_;
//   HandlerRequestId hander_request_id_;
// };

template <typename TResponse, typename TRequest>
class AsyncClientCall : public AsyncClientCallBase {
 public:
  using TResponseReader =
      std::unique_ptr<grpc::ClientAsyncResponseReader<TResponse>>;
  using PrepareAsyncFun =
      std::function<TResponseReader(const TRequest& request)>;
  AsyncClientCall(AsyncGrpcResponseHandler response_handler,
                  HandlerRequestId handler_id,
                  std::shared_ptr<GrpcClientContext> context,
                  PrepareAsyncFun prepare_async_fun)
      : AsyncClientCallBase(context),
        grpc_response_handler_(std::move(response_handler)),
        hander_request_id_(handler_id),
        prepare_async_fun_(prepare_async_fun) {}
  AsyncClientCall(AsyncGrpcRawResponseHandler<TResponse> response_handler,
                  HandlerRequestId handler_id,
                  std::shared_ptr<GrpcClientContext> context,
                  PrepareAsyncFun prepare_async_fun,
                  bool serialization_json)
      : AsyncClientCallBase(context),
        grpc_raw_response_handler_(std::move(response_handler)),
        hander_request_id_(handler_id),
        prepare_async_fun_(prepare_async_fun),
        serialization_json_(serialization_json) {}

  virtual void HandleResponse() override {
    //LOG(INFO) << "response hander_request_id_: " << static_cast<int>(hander_request_id_);
    if (serialization_json_) {
      std::string grpc_response;
      if (status.ok()) {
        google::protobuf::util::MessageToJsonString(response_, &grpc_response);
      } else {
        LOG(ERROR)
            << "AsyncClientCall HandleResponse failed,hander_request_id_:"
            << static_cast<int>(hander_request_id_)
            << "grpc error:" << status.error_details()
            << ",message:" << status.error_message();
      }
      std::move(grpc_response_handler_)
          .Run(hander_request_id_, status, context_, grpc_response);
    } else {
      std::move(grpc_raw_response_handler_)
          .Run(hander_request_id_, status, context_, response_);
    }
  }

  void Call(const std::string& param) {
    TRequest request;
    google::protobuf::util::JsonParseOptions options;
    options.ignore_unknown_fields = true;
    google::protobuf::util::JsonStringToMessage(param, &request, options);

    response_reader_ = std::move(prepare_async_fun_(request));
    if (response_reader_) {
      response_reader_->StartCall();
      response_reader_->Finish(&response_, &status, (void*)this);
    } else {
      LOG(ERROR) << "Call response reader null!";
    }
  }
  void RawCall(TRequest request) {
    response_reader_ = std::move(prepare_async_fun_(request));
    if (response_reader_) {
      response_reader_->StartCall();
      response_reader_->Finish(&response_, &status, (void*)this);
    } else {
      LOG(ERROR) << "RawCall response reader null!";
    }
  }

 private:
  AsyncGrpcResponseHandler grpc_response_handler_;
  AsyncGrpcRawResponseHandler<TResponse> grpc_raw_response_handler_;
  HandlerRequestId hander_request_id_;
  TResponse response_;
  TResponseReader response_reader_;
  PrepareAsyncFun prepare_async_fun_;
  bool serialization_json_{true};
};

class GrpcStubBase {
 public:
  GrpcStubBase(HandlerRequestId handler_id,
               const std::multimap<std::string, std::string>& header);
  void InitCompletionQueue(grpc::CompletionQueue* grpc_completion_queue);

  /// @brief 将执行具体的到grpc的请求,包含参数转换到调用具体接口等
  /// @return 是否被处理,不表示处理结果成功或失败
  virtual bool Handler(const std::string& param);
  virtual bool AsyncHandler(const std::string& param,
                            AsyncGrpcResponseHandler response_handler) = 0;
  virtual ~GrpcStubBase();

  // function
 protected:
  virtual void AsyncInitStub(GetStubCallbackCallback);
  // attribute
 protected:
  // 请求
  HandlerRequestId handler_id_;
  // http的请求头
  std::multimap<std::string, std::string> request_header_;
  std::shared_ptr<GrpcHoldStateChannel> state_channel_;

  // grpc 请求的context
  std::shared_ptr<GrpcClientContext> client_context_;
  // 服务注册的名称
  std::string service_package_name_;
  // 异步完成端口从外部传入
  grpc::CompletionQueue* grpc_completion_queue_;
};  // GrpcStubBase
}  // namespace nas

#endif  // NAS_COMMON_GRPC_STUB_GRPC_STUB_BASE_H_