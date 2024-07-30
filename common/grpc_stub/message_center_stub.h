/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-02-01 18:02:37
 */

#ifndef NAS_COMMON_GRPC_STUB_MESSAGE_CENTER_STUB_H_
#define NAS_COMMON_GRPC_STUB_MESSAGE_CENTER_STUB_H_

#include <map>
#include <string>
#include <vector>
#include <queue>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"
#include "base/json/json_writer.h"
#include "base/threading/thread.h"

#include "grpc_stub_base.h"
#include "nas/messages_center/protos/messages.grpc.pb.h"
#include "grpc_hold_state_channel.h"

namespace nas {

class MessageCenterStub : public base::RefCountedThreadSafe<MessageCenterStub>,
                          public GrpcStubBase {
  public:
    MessageCenterStub(HandlerRequestId handler_id,
                  const std::multimap<std::string, std::string>& header);

    virtual bool AsyncHandler(const std::string& param,
                            AsyncGrpcResponseHandler grpc_response_handler) override;
    
  protected:
   virtual void AsyncInitStub(GetStubCallbackCallback) override;
   void Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler);

  //Attrbute
  private:
    // 消息中心的stub 端
    std::unique_ptr<messages_center::MessageCenter::Stub> message_center_stub_;
    
    friend class base::RefCountedThreadSafe<MessageCenterStub>;
    virtual ~MessageCenterStub();
}; // MessageCenterStub
}  // namespace nas

#endif  // NAS_COMMON_GRPC_STUB_MESSAGE_CENTER_STUB_H_