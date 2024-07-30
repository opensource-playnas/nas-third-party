#include "oss_manager.h"

#include "alibabacloud/oss/OssClient.h"

#include "base/logging.h"

namespace nas {

OssMgr::OssMgr() {
  AlibabaCloud::OSS::InitializeSdk();
}

OssMgr::~OssMgr() {
  AlibabaCloud::OSS::ShutdownSdk();
}

void OssMgr::SetOssInfo(const std::string& access_key_id,
                        const std::string& access_key_secret,
                        const std::string& endpoint,
                        const std::string& security_token) {
  access_key_id_ = access_key_id;
  access_key_secret_ = access_key_secret;
  endpoint_ = endpoint;
  security_token_ = security_token;
}

bool OssMgr::Upload(const std::string& bucket_name,
                    const std::string& relative_path_file,
                    const std::string& file_path) {
  bool result = false;
  if (!AlibabaCloud::OSS::IsSdkInitialized()) {
    std::cout << "sdk init failed";
    return result;
  }

  AlibabaCloud::OSS::ClientConfiguration conf;
  AlibabaCloud::OSS::OssClient client(
      endpoint_, access_key_id_, access_key_secret_, security_token_, conf);

  std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>(
      file_path.c_str(), std::ios::in | std::ios::binary);
  AlibabaCloud::OSS::PutObjectRequest request(bucket_name, relative_path_file,
                                              content);
  //   AlibabaCloud::OSS::TransferProgress progress_callback = {
  //       ProgressCallback, call_back ? &call_back : nullptr};
  //   request.setTransferProgress(progress_callback);
  auto outcome = client.PutObject(request);
  if (!outcome.isSuccess()) {
    LOG(WARNING) << "PutObject fail"
                 << ",code:" << outcome.error().Code()
                 << ",message:" << outcome.error().Message()
                 << ",requestId:" << outcome.error().RequestId() << std::endl;
  } else {
    result = true;
  }
  return result;
}

};  // namespace nas