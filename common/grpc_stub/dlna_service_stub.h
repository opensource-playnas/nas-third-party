/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: fengbangyao@ludashi.com
* @Date: 2023-02-10 13:47:51
*/


#ifndef NAS_COMMON_GRPC_STUB_DLNA_SERVICE_STUB_H_
#define NAS_COMMON_GRPC_STUB_DLNA_SERVICE_STUB_H_

#include <map>
#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"

#include "nas/dlna_server/protos/dlna_server.grpc.pb.h"
#include "grpc_stub_base.h"
#include "grpc_hold_state_channel.h"
namespace nas {

class DlnaServerStub : public base::RefCountedThreadSafe<DlnaServerStub>,
                       public GrpcStubBase {
                        
 public:
  DlnaServerStub(HandlerRequestId handler_id,
                  const std::multimap<std::string, std::string>& header);
  virtual bool AsyncHandler(const std::string& param,
                            AsyncGrpcResponseHandler grpc_response_handler) override;
 protected:  
   virtual void AsyncInitStub(GetStubCallbackCallback) override;
   void Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler);
 private:
  // 媒体服务的stub 端
  std::unique_ptr<dlna::v1::DlnaService::Stub> dlna_service_stub_;
  friend class base::RefCountedThreadSafe<DlnaServerStub>;
  virtual ~DlnaServerStub(){};                      
}; // DlnaServerStub

}  // namespace nas
#endif  // NAS_COMMON_GRPC_STUB_DLNA_SERVICE_STUB_H_