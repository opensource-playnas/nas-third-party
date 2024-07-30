/*
 * @Description: 用于给 MessageCenter 发送消息
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: guopengwei@ludashi.com
 * @Date: 2022-09-21 17:02:54
 */

#ifndef NAS_COMMON_SEND_MESSAHE_H_
#define NAS_COMMON_SEND_MESSAHE_H_

#include <memory>

#include <base/memory/singleton.h>
#include <base/task/thread_pool.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "nas/common/service_manager/services_mgr_stub.h"

#include "context_field.h"
#include "nas/messages_center/protos/messages.grpc.pb.h"

namespace nas {
using ReplyCallback = base::OnceCallback<void(const std::string&)>;
class SendToMessageCenter {
 public:
  static SendToMessageCenter* GetInstance();
  SendToMessageCenter(const SendToMessageCenter&) = delete;
  SendToMessageCenter& operator=(const SendToMessageCenter&) = delete;

  // 区分不同套件
  void Init(const std::string& app,
            std::shared_ptr<ServicesMgrStub> services_mgr_stub);
  void UnInit();
  void AsyncPublish(const std::string& content_json,
                    const ContextField& context_field);
  void AsyncPublishByXEndpointId(const std::string& content_json,
                                 const ContextField& context_field);
  void AsyncPublishByUserId(const std::string& content_json,
                            const ContextField& context_field);

  // 数据推送到消息中心后会被存储下来,一定是异步的
  void PublishToMessageCenter(const std::string& title,
                              const std::string& sub_title,
                              int template_id,
                              const std::string& type,
                              const std::string& action,
                              const std::string& status,
                              int level,
                              const std::string& content_json,
                              const ContextField& context_field);

  void HSet(const std::string& key,
            const std::string& field,
            const std::string& value,
            int32_t expire);
  void HGet(const std::string& key,
            const std::string& field,
            ReplyCallback callback);

 private:
  SendToMessageCenter();
  ~SendToMessageCenter();
  friend struct base::DefaultSingletonTraits<SendToMessageCenter>;

  // 创建消息中心Stub
  bool CreateMessageCenterStub();

  void Publish(const std::string& content_json,
               const ContextField& context_field,
               const std::string& channel);

  std::unique_ptr<messages_center::MessageCenter::Stub> stub_ = nullptr;

  // 需要保证我们发送的通知任务是有序的
  // scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner_ =
  //     base::ThreadPool::CreateSequencedTaskRunner(
  //         {base::TaskPriority::BEST_EFFORT});
  std::shared_ptr<base::Thread> work_thread_;
  std::string app_;
  std::shared_ptr<ServicesMgrStub> services_mgr_stub_;
};

};      // end namespace nas
#endif  // NAS_COMMON_SEND_MESSAHE_H_