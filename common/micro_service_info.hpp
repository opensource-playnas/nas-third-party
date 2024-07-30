/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-02-03 19:38:49
 */

#ifndef NAS_COMMON_MICRO_SERVICE_INFO_HPP_
#define NAS_COMMON_MICRO_SERVICE_INFO_HPP_

namespace nas {
static const char* kServiceManageName = "service_manager";
static const char* kGatewayServiceName = "gateway";
static const char* kAuthServiceName = "auth_service.v1";
static const char* kFileServiceName = "file.v1";
static const char* kMessageCenterName = "messages_center.v1";
static const char* kMediaService = "media.v1";
static const char* kDlnaService = "dlna.v1";
static const char* kImageServiceName = "image.v1";
static const char* kDownloadServiceName = "download.v1";
static const char* kUpdateServiceName = "update.v1";
static const char* kSystemInfoServiceName = "system_info.v1";
static const char* kPictureServiceName = "picture.v1";
static const char* kGrpcProxyServiceName = "grpc_proxy.v1";

static const char* kNginxName = "nginx";
static const char* kCloudConnectorName = "cloud.connector";
static const char* kRedisName = "redis";
static const char* kPhpCgiName = "php-cgi";

namespace suitename {
static const char kGatewaySuiteName[] = "gateway";
static const char kFileServerSuiteName[] = "file_station";
static const char kDwonloadServerSuiteName[] = "download_server";
static const char kUpdateServerSuiteName[] = "update_server";
static const char kMediaServerSuiteName[] = "media_server";
static const char kImageServerSuiteName[] = "image_server";
static const char kAuthServiceSuiteName[] = "auth_server";
static const char kMessageCenterSuiteName[] = "message_center";
static const char kServiceManageSuiteName[] = "service_manager";
static const char kGrpcProxySuiteName[] = "grpc_proxy";
static const char kSystemInfoSuiteName[] = "system_info";
static const char kDlnaServerSuiteName[] = "dlna_server";


}  // namespace suitename

}  // namespace nas

#endif  // NAS_COMMON_MICRO_SERVICE_INFO_HPP_