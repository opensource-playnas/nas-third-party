/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-04-18 19:57:13
 */
#include "parse_mgr.h"

#include "base/strings/escape.h"

#include "download_error_desc.h"
#include "nas/common/path/file_path_unit.h"
#include "task/bt_magnet_task.h"
#include "task/bt_torrent_task.h"
#include "task/curl_task.h"
#include "task/mule_task.h"
#include "task_factory.h"
#include "utils/utils.h"


namespace nas {
ParseManager::ParseManager() {
}

ParseManager::~ParseManager() {}

DownloadErrorCode ParseManager::ParseLink(
    const download::v1::ParseLinkRequest* request,
    download::v1::ParseLinkReply* response,
    const ContextField& context_field) {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  TaskParamPtr task_param = std::make_shared<TaskParam>();
  task_param->url = request->source();
  task_param->is_parse_task = true;

  do {
    if (!nas::utils::IsValid(task_param->url)) {
      err = DownloadErrorCode::kCurlUrlInvalid;
      break;
    }

    TaskBasePtr task = TaskFactory::CreateTask(task_param, context_field);
    if (!task) {
      err = DownloadErrorCode::kCreateTaskFailed;
      break;
    }
    ParseResult result;
    int ret = task->SyncParse(&result);
    if (ret) {
      err = DownloadErrorCode::kParseCurlUrlFailed;
    }
    download_data_struct::SourceParseInfo* info = response->mutable_file_info();
    info->set_file_name(result.file_info.file_name);
    info->set_file_size(std::to_string(result.file_info.file_size));
    info->set_file_type(std::to_string(
        nas::path_unit::GetFileType(result.file_info.file_name)));
  } while (0);

  return err;
}

DownloadErrorCode ParseManager::ParseTorrent(
    const download::v1::ParseTorrentRequest* request,
    download::v1::ParseTorrentReply* response,
    const ContextField& context_field) {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  TorrentTaskParamPtr task_param = std::make_shared<TorrentTaskParam>();
  task_param->url = base::UnescapeBinaryURLComponent(
      request->source(), base::UnescapeRule::NORMAL);
  task_param->is_parse_task = true;

  do {
    TaskBasePtr task = TaskFactory::CreateTask(task_param, context_field);
    if (!task) {
      err = DownloadErrorCode::kCreateTaskFailed;
      break;
    }

    int ret = task->SyncParse(NULL);
    err = DownloadErrorCode(ret);

    std::string parse_result = task->GetTaskInfo()->GetTaskBaseInfo()->GetFileList();
    response->set_parse_result(std::move(parse_result));

  } while (0);

  return err;
}

}  // namespace nas
