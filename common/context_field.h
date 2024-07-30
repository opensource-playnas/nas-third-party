/*
 * @Description: 解析 ServerContext 中的字段
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: guopengwei@ludashi.com
 * @Date: 2022-09-19 14:43:01
 */

#ifndef NAS_COMMON_CONTEXT_FIELD_H_
#define NAS_COMMON_CONTEXT_FIELD_H_

#include <string>

#include <grpcpp/impl/codegen/server_context.h>

namespace nas {

class ContextField {
 public:
  explicit ContextField(grpc::ServerContext* context);
  explicit ContextField(const std::string& nas_token,
                        const std::string& user_id,
                        const std::string& call_id);
  ContextField() = default;

  void Isvalid() const;

 public:
  std::string token() const;
  std::string nas_token() const;
  std::string user_id() const;
  std::string call_id() const;
  std::string authorization() const;
  std::string x_endpoint_id() const;
  std::string client_type() const;

  void set_x_endpoint_id(const std::string& value) { x_endpoint_id_ = value; }
  void set_call_id(const std::string& value) { call_id_ = value; }
  void set_user_id(const std::string& value) { user_id_ = value; }

 private:
  std::string token_;
  std::string nas_token_;
  std::string user_id_;
  std::string call_id_;
  std::string authorization_;
  std::string client_type_;

  // 标识一个连接端的唯一id
  std::string x_endpoint_id_;
};

};  // namespace nas

#endif  // NAS_COMMON_CONTEXT_FIELD_H_
