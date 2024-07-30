#include "send_to_message_center.h"

#include <base/logging.h>
#include <base/test/bind.h>
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/threading/thread.h"
#include "google/protobuf/util/json_util.h"

#include "nas/common/micro_service_info.hpp"

namespace nas {
SendToMessageCenter* SendToMessageCenter::GetInstance() {
  return base::Singleton<SendToMessageCenter>::get();
}

SendToMessageCenter::SendToMessageCenter() {}

void SendToMessageCenter::Init(
    const std::string& app,
    std::shared_ptr<ServicesMgrStub> services_mgr_stub) {
  services_mgr_stub_ = services_mgr_stub;
  app_ = app;

  CreateMessageCenterStub();
  work_thread_ = std::make_shared<base::Thread>("SendToMessageCenter");
  if (work_thread_ && !work_thread_->IsRunning()) {
    work_thread_->StartWithOptions(
        base::Thread::Options{base::MessagePumpType::IO, 0});
  }
}

SendToMessageCenter::~SendToMessageCenter() {}

void SendToMessageCenter::UnInit() {
  if (work_thread_) {
    work_thread_->Stop();
  }
}

bool SendToMessageCenter::CreateMessageCenterStub() {
  service_manager::v1::GrpcServiceInfo grpc_info;
  services_mgr_stub_->QueryGrpcServices(kMessageCenterName, &grpc_info);
  std::string message_center_address = grpc_info.ip_port();

  LOG(INFO) << "message_center_address:" << message_center_address;
  if (!message_center_address.empty()) {
    std::shared_ptr<grpc::Channel> channel{grpc::CreateChannel(
        message_center_address, grpc::InsecureChannelCredentials())};

    stub_ = messages_center::MessageCenter::NewStub(channel);
  } else {
    LOG(INFO) << "message_center_address: is empty";
  }

  return stub_ != nullptr;
}

void SendToMessageCenter::Publish(const std::string& content_json,
                                  const ContextField& context_field,
                                  const std::string& channel) {
  if (!work_thread_->task_runner()->RunsTasksInCurrentSequence()) {
    work_thread_->task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(&SendToMessageCenter::Publish, base::Unretained(this),
                       content_json, context_field, channel));
    return;
  }
  if (!stub_ && !CreateMessageCenterStub()) {
    LOG(ERROR) << "stub is null";
    return;
  }

  messages_center::PubSub request;
  std::string* chanel = request.mutable_pubchannel()->mutable_channel();
  *chanel = channel;

  messages_center::PubMessage* pubmessage = request.mutable_message();
  pubmessage->set_app(app_);
  pubmessage->set_operator_id(context_field.call_id());

  google::protobuf::util::JsonParseOptions options;
  options.ignore_unknown_fields = true;
  google::protobuf::util::JsonStringToMessage(
      content_json, pubmessage->mutable_content(), options);

  messages_center::MessageId reply;
  grpc::ClientContext context;
  grpc::Status status = stub_->Publish(&context, request, &reply);

  if (status.ok()) {
    LOG(INFO) << "Publish success. channel: " << channel;
    return;
  }
  LOG(ERROR) << "Publish failed.";

  // 失败重置stub
  stub_.reset();
  work_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&SendToMessageCenter::Publish, base::Unretained(this),
                     content_json, context_field, channel));
}

void SendToMessageCenter::AsyncPublish(const std::string& content_jon_str,
                                       const ContextField& context_field) {
  if (!work_thread_) {
    LOG(INFO) << "AsyncPublish work_thread null";
    return;
  }

  work_thread_->task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&SendToMessageCenter::Publish,
                                base::Unretained(this), content_jon_str,
                                context_field, context_field.nas_token()));
}

void SendToMessageCenter::AsyncPublishByXEndpointId(
    const std::string& content_json,
    const ContextField& context_field) {
  work_thread_->task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&SendToMessageCenter::Publish,
                                base::Unretained(this), content_json,
                                context_field, context_field.x_endpoint_id()));
}

void SendToMessageCenter::AsyncPublishByUserId(
    const std::string& content_json,
    const ContextField& context_field) {
  if (!work_thread_) {
    return;
  }

  work_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&SendToMessageCenter::Publish, base::Unretained(this),
                     content_json, context_field, context_field.user_id()));
}

void SendToMessageCenter::PublishToMessageCenter(
    const std::string& title,
    const std::string& sub_title,
    int template_id,
    const std::string& type,
    const std::string& action,
    const std::string& status,
    int level,
    const std::string& content_json,
    const ContextField& context_field) {
  if (!work_thread_->task_runner()->RunsTasksInCurrentSequence()) {
    work_thread_->task_runner()->PostTask(
        FROM_HERE, base::BindOnce(&SendToMessageCenter::PublishToMessageCenter,
                                  base::Unretained(this), title, sub_title,
                                  template_id, type, action, status, level,
                                  content_json, context_field));
    return;
  }
  if (!stub_ && !CreateMessageCenterStub()) {
    LOG(ERROR) << "stub is null";
    return;
  }

  messages_center::UserMessage request;
  request.set_user(context_field.user_id());

  messages_center::MessageInfo* msg_info = request.mutable_info();
  msg_info->set_app(app_);
  msg_info->set_title(title);
  msg_info->set_sub_title(sub_title);
  msg_info->set_type(type);
  msg_info->set_action(action);
  msg_info->set_status(status);
  msg_info->set_level(static_cast<messages_center::MessageLevel>(level));
  messages_center::MessagePayload* play_load = msg_info->mutable_payload();
  play_load->set_template_id(template_id);

  google::protobuf::util::JsonParseOptions options;
  options.ignore_unknown_fields = true;
  google::protobuf::util::JsonStringToMessage(
      content_json, play_load->mutable_detail(), options);

  messages_center::MessageResponse reply;
  grpc::ClientContext context;
  LOG(INFO) << "publish to messagecenter msg: " << context_field.call_id()
            << ", user_id: " << context_field.user_id()
            << ", token: " << context_field.nas_token();
  grpc::Status grpc_status =
      stub_->PublishToMessageCenter(&context, request, &reply);

  if (grpc_status.ok()) {
    LOG(INFO) << "PublishToMessageCenter success.";
    return;
  }
  LOG(ERROR) << "PublishToMessageCenter failed.";

  // 失败重置stub
  stub_.reset();
  work_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&SendToMessageCenter::PublishToMessageCenter,
                     base::Unretained(this), title, sub_title, template_id,
                     type, action, status, level, content_json, context_field));
}

}  // namespace nas