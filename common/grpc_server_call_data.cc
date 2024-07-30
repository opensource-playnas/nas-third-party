// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author heyiqian@ludashi.com
// date 2023-07-13 16:06:50

#include "grpc_server_call_data.h"

namespace nas {
void HandleRpcs(grpc::ServerCompletionQueue* cq) {
  GPR_ASSERT(cq);
  void* tag = nullptr;
  bool ok = false;
  while (true) {
    if (!cq->Next(&tag, &ok)) {
        break;
    }

    if(ok){
      static_cast<GrpcServerCallData*>(tag)->Proceed();
    }
    
  }
}

// 用完把之前的删掉 
void HandleRpcRequests(grpc::ServerCompletionQueue* cq) {
  GPR_ASSERT(cq);
  void* tag = nullptr;
  bool ok = false;
  while (true) {
    if (!cq->Next(&tag, &ok)) {
        break;
    }
    if(ok){    
      static_cast<TagHandler*>(tag)->Proceed();
    }
  }
}
}  // namespace nas