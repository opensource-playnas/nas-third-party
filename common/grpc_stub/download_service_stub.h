/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: wanyan@ludashi.com
* @Date: 2023-03-17 11:02:41
*/

#ifndef NAS_COMMON_GRPC_STUB_DOWNLOAD_SERVICE_STUB_H_
#define NAS_COMMON_GRPC_STUB_DOWNLOAD_SERVICE_STUB_H_

#include <map>
#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"

#include "nas/download_service/download_server/protos/download_server.grpc.pb.h"
#include "grpc_stub_base.h"
#include "grpc_hold_state_channel.h"
namespace nas {

class DownloadServerStub : public base::RefCountedThreadSafe<DownloadServerStub>,
                       public GrpcStubBase {
                        
 public:
  DownloadServerStub(HandlerRequestId handler_id,
                  const std::multimap<std::string, std::string>& header);
  virtual bool AsyncHandler(const std::string& param,
                            AsyncGrpcResponseHandler grpc_response_handler) override;
 protected: 
   virtual void AsyncInitStub(GetStubCallbackCallback) override;
   void Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler);
 private:
  // 远程下载服务的stub 端
  std::unique_ptr<download::v1::DownloadService::Stub> download_service_stub_;
  friend class base::RefCountedThreadSafe<DownloadServerStub>;
  virtual ~DownloadServerStub(){};                   
}; // DownloadServerStub

}  // namespace nas
#endif  // NAS_COMMON_GRPC_STUB_DOWNLOAD_SERVICE_STUB_H_