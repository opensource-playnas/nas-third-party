/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: wanyan@ludashi.com
* @Date: 2023-05-22 15:01:11
*/
#ifndef NAS_COMMON_GRPC_BASE_SERVICE_GRPC_BASE_INTERFACE_H_
#define NAS_COMMON_GRPC_BASE_SERVICE_GRPC_BASE_INTERFACE_H_

namespace nas {
class GrpcBaseServiceInterface {
 public:
  virtual ~GrpcBaseServiceInterface() {}
  virtual void Stop() = 0;
  virtual void Reset(int level) = 0;

};
}

#endif // NAS_COMMON_GRPC_BASE_SERVICE_GRPC_BASE_INTERFACE_H_
