/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-04-11 15:57:30
 */

#ifndef NAS_COMMON_AUTH_SERVICE_CLIENT_H_
#define NAS_COMMON_AUTH_SERVICE_CLIENT_H_

#include <memory>

#include <base/memory/singleton.h>
#include <base/task/thread_pool.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "nas/auth_service/protos/auth_service.grpc.pb.h"
#include "nas/common/context_field.h"
#include "nas/common/service_manager/services_mgr_stub.h"

namespace nas {

using VerifyNasTokenCallback =
    base::OnceCallback<void(bool is_pass, const std::string& user_id)>;
class AuthServiceClient {
 public:
  static AuthServiceClient* GetInstance();
  AuthServiceClient(const AuthServiceClient&) = delete;
  AuthServiceClient& operator=(const AuthServiceClient&) = delete;

  // 区分不同套件
  void Init(const std::string& app,
            std::shared_ptr<ServicesMgrStub> services_mgr_stub);
  void UnInit();
  bool CreateAuthServiceStub();

  bool GetAllUser(auth_service::v1::UserInfoList* user_list);

  bool GetNasBaseInfo(auth_service::v1::NasBaseInfo* info);
  bool CleanNasBaseInfo();
  bool UpdateNasBaseInfo(auth_service::v1::NasBaseInfo& info);
  void SetAddress(const std::string& address) { address_ = address; }

 private:
  AuthServiceClient();
  ~AuthServiceClient();
  friend struct base::DefaultSingletonTraits<AuthServiceClient>;

 private:
  std::shared_ptr<base::Thread> work_thread_;
  std::string app_;
  std::string address_;  // 当 没有 services_mgr_stub_时,可以直接用地址
  std::unique_ptr<auth_service::v1::AuthService::Stub> stub_ = nullptr;
  std::shared_ptr<ServicesMgrStub> services_mgr_stub_;

};  // class AuthServiceClient

}  // namespace nas

#endif  // NAS_COMMON_AUTH_SERVICE_CLIENT_H_
