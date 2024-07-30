#include "context_field.h"

namespace nas {

std::string ContextField::nas_token() const {
  return nas_token_;
}

std::string ContextField::token() const {
  return token_;
}

std::string ContextField::user_id() const {
  return user_id_;
}

std::string ContextField::call_id() const {
  return call_id_;
}

std::string ContextField::authorization() const {
  return authorization_;
}

std::string ContextField::x_endpoint_id() const {
  return x_endpoint_id_;
}

std::string ContextField::client_type() const {
  return client_type_;
}


ContextField::ContextField(grpc::ServerContext* context) {
  if (context != nullptr) {
    const std::multimap<grpc::string_ref, grpc::string_ref>& client_meta =
        context->client_metadata();
    for (auto iter : client_meta) {
      if (iter.first == "nas_token") {
        nas_token_ = std::string(iter.second.begin(), iter.second.end());
      } else if (iter.first == "token") {
        token_ = std::string(iter.second.begin(), iter.second.end());
      } else if (iter.first == "nas_user_id") {
        user_id_ = std::string(iter.second.begin(), iter.second.end());
      } else if (iter.first == "nas_call_id") {
        call_id_ = std::string(iter.second.begin(), iter.second.end());
      } else if (iter.first == "authorization") {
        authorization_ = std::string(iter.second.begin(), iter.second.end());
      } else if (iter.first == "x-endpoint-id") {
        x_endpoint_id_ = std::string(iter.second.begin(), iter.second.end());
      } else if (iter.first == "x-client-type") {
        client_type_ = std::string(iter.second.begin(), iter.second.end());
      }
    }
  }
}

ContextField::ContextField(const std::string& nas_token,
                           const std::string& user_id,
                           const std::string& call_id) {
  nas_token_ = nas_token;
  user_id_ = user_id;
  call_id_ = call_id;
}

}  // namespace nas