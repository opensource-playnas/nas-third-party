#include "task_factory.h"

#include "base/strings/string_util.h"
#include "download_helper/download_interface_helper.h"
#include "nas/common/path/file_path_unit.h"
#include "task/bt_magnet_task.h"
#include "task/bt_torrent_task.h"
#include "task/curl_task.h"
#include "task/mule_task.h"
#include "utils/utils.h"

namespace nas {
TaskFactory::TaskFactory() {}

TaskFactory::~TaskFactory() {}

TaskBasePtr TaskFactory::CreateTask(TaskParamPtr task_param,
                                    const ContextField& context_field) {
  TaskBasePtr task = nullptr;
  DownloadType type = GetDownloadType(task_param->url);
  task_param->type = type;
  switch (type) {
    case DownloadType::kEd2k: {
      if (!DownloadInterfaceHelper::GetInstance()->IsLoadedNetMule()) {
        LOG(ERROR) << "load netmule failed, so cannot create task";
        break;
      }
      task = std::make_shared<MuleTask>(task_param, context_field);
    } break;
    case DownloadType::kTorrent: {
      if (!DownloadInterfaceHelper::GetInstance()->IsLoadedNetBt()) {
        LOG(ERROR) << "load netbt failed, so cannot create task";
        break;
      }
      task = std::make_shared<BtTorrentTask>(task_param, context_field);
    } break;
    case DownloadType::kMagnet: {
      if (!DownloadInterfaceHelper::GetInstance()->IsLoadedNetBt()) {
        LOG(ERROR) << "load netbt failed, so cannot create task";
        break;
      }
      task = std::make_shared<BtMagnetTask>(task_param, context_field);
    } break;
    case DownloadType::kHttp:
    case DownloadType::kHttps:
    case DownloadType::kFtp:
    case DownloadType::kFtps:
    case DownloadType::kSftp: {
      if (!DownloadInterfaceHelper::GetInstance()->IsLoadedNetCurl()) {
        LOG(ERROR) << "load netcurl failed, so cannot create task";
        break;
      }
      task = std::make_shared<CurlTask>(task_param, context_field);
      break;
    }
    default:
      LOG(ERROR) << "get download type is unknown!";
      break;
  }
  return task;
}

DownloadType TaskFactory::GetDownloadType(const std::string& url) {
  base::StringPiece s = url.c_str();
  std::string str = base::ToLowerASCII(s);
  DownloadType type = DownloadType::kUnknown;
  if (str.substr(0, 7) == "ed2k://") {
    type = kEd2k;
  } else if (base::PathExists(nas::path_unit::BaseFilePathFromU8(url)) &&
             url.find(".torrent") != std::string::npos) {
    type = kTorrent;
  } else if (str.substr(0, 20) == "magnet:?xt=urn:btih:") {
    type = kMagnet;
  } else if (str.substr(0, 7) == "http://") {
    type = kHttp;
  } else if (str.substr(0, 8) == "https://") {
    type = kHttps;
  } else if (str.substr(0, 6) == "ftp://") {
    type = kFtp;
  } else if (str.substr(0, 7) == "ftps://") {
    type = kFtps;
  } else if (str.substr(0, 7) == "sftp://") {
    type = kSftp;
  }

  return type;
}

TorrentTaskParamPtr TaskFactory::GetTorrentTaskParam(
    TaskParamPtr task_param,
    const ContextField& context_field) {
  if (context_field.nas_token().empty()) {
    TorrentTaskParamPtr torrent_param = std::make_shared<TorrentTaskParam>();
    torrent_param->id = task_param->id;
    torrent_param->url = task_param->url;
    torrent_param->save_path = task_param->save_path;
    torrent_param->file_name = task_param->file_name;
    torrent_param->type = task_param->type;
    return torrent_param;
  } else {
    TorrentTaskParamPtr param =
        std::static_pointer_cast<TorrentTaskParam>(task_param);
    return param;
  }
}

}  // namespace nas
