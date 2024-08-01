/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:52:21
 */
#include "download_mgr.h"

#include "base/base64.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/rand_util.h"
#include "base/strings/escape.h"
#include "base/strings/string_util.h"

#include "config/system_cloud_config.h"
#include "db/download_store_helper.h"
#include "download_error_desc.h"
#include "download_helper/download_interface_helper.h"
#include "nas/common//path/file_path_unit.h"
#include "nas/common/nas_config.h"
#include "nas/common/nas_thread.h"
#include "task/bt_magnet_task.h"
#include "task/bt_task_base.h"
#include "task/bt_torrent_task.h"
#include "task/curl_task.h"
#include "task/mule_task.h"
#include "task_factory.h"
#include "utils/utils.h"

namespace nas {
static const char* kDownloadMgrThreadName = "download_mgr_thread";

DownloadManager::DownloadManager() {
  work_thread_ = std::make_shared<NasThread>(kDownloadMgrThreadName,
                                             base::MessagePumpType::IO);
}

DownloadManager::~DownloadManager() {}

void DownloadManager::Init() {
  DownloadInterfaceHelper::GetInstance()->Init();
  DownloadStoreHelper::GetInstance()->Init();
  SystemCloudConfig::GetInstance()->Init();
  // 恢复数据库数据，此时不需要主动推送数据，只有当前端掉grpc接口后再去推送
  ResumeTasksFromDb();
}

void DownloadManager::UnInit() {
  LOG(WARNING) << __func__;
  if (work_thread_) {
    work_thread_->Stop();
  }

  for (auto& [user, user_self_mgr] : user_self_mgr_list_) {
    user_self_mgr->Stop();
  }

  for (auto user_thread : user_thread_list_) {
    if (user_thread) {
      user_thread->Stop();
    }
  }

  DownloadStoreHelper::GetInstance()->UnInit();
  DownloadInterfaceHelper::GetInstance()->UnInit();
  LOG(INFO) << __func__ << " finished";
}

void DownloadManager::Reset(base::WaitableEvent& wait_event) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE, base::BindOnce(&DownloadManager::Reset, shared_from_this(),
                                  std::ref(wait_event)));
    return;
  }

  base::WaitableEvent user_mgr_wait_event;
  LOG(WARNING) << __func__;
  for (auto& [user, user_self_mgr] : user_self_mgr_list_) {
    user_self_mgr->Reset(user_mgr_wait_event);
  }

  if (user_self_mgr_list_.size() > 0) {
    user_mgr_wait_event.Wait();
  }
  user_self_mgr_list_.clear();
  user_thread_list_.clear();
  DownloadStoreHelper::GetInstance()->Reset();
  base::DeletePathRecursively(nas::utils::GetTorrentFileSavePath());
  base::PlatformThread::Sleep(base::Seconds(1));
  wait_event.Signal();
}

void DownloadManager::CreateLinkDownloadTask(
    const download::v1::CreateLinkDownloadTaskRequest* request,
    const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    auto copy_request =
        std::make_unique<download::v1::CreateLinkDownloadTaskRequest>(*request);
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(&DownloadManager::CreateLinkDownloadTask,
                       shared_from_this(), base::Owned(copy_request.release()),
                       context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    user_self_mgr = AddUser(context_field);
  }

  if (user_self_mgr) {
    user_self_mgr->CreateLinkTask(request, context_field);
  }
}

void DownloadManager::CreateTorrentDownloadTask(
    const download::v1::CreateTorrentDownloadTaskRequest* request,
    const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    auto copy_request =
        std::make_unique<download::v1::CreateTorrentDownloadTaskRequest>(
            *request);
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(&DownloadManager::CreateTorrentDownloadTask,
                       shared_from_this(), base::Owned(copy_request.release()),
                       context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    user_self_mgr = AddUser(context_field);
  }

  if (user_self_mgr) {
    user_self_mgr->CreateTorrentTask(request, context_field);
  }
}

std::string DownloadManager::UploadTorrentFile(
    const download::v1::UploadTorrentFileRequest* request) {
  std::string torrent_path;
  do {
    std::string file_content;
    std::string recv_file_content = request->file_content();
    base::Base64Decode(recv_file_content, &file_content);
    if (file_content.empty()) {
      LOG(ERROR) << "QueryTorrentFileInfo::get torrent content is null";
      break;
    }

    base::FilePath dir_path = nas::utils::GetUploadTorrentFileCachePath();

    base::FilePath file_name =
        path_unit::BaseFilePathFromU8(request->file_name());
    base::FilePath file_path = dir_path.Append(file_name);
    base::File file(file_path,
                    base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);

    if (!file.IsValid()) {
      LOG(ERROR) << "Error: Unable to open file " << file_path;
      break;
    }

    int bytes_written =
        file.WriteAtCurrentPos(file_content.data(), file_content.size());
    if (bytes_written < 0 ||
        static_cast<size_t>(bytes_written) != file_content.size()) {
      LOG(ERROR) << "Error: Failed to write data to " << file_path;
      break;
    }
    file.Close();

    torrent_path = path_unit::BaseFilePathToU8(file_path);
    torrent_path = path_unit::UnifiedSlash(torrent_path);
  } while (0);

  return torrent_path;
}

void DownloadManager::Pause(const download::v1::PauseRequest* request,
                            const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    auto copy_request = std::make_unique<download::v1::PauseRequest>(*request);
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(&DownloadManager::Pause, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    return;
  }

  user_self_mgr->Pause(request, context_field);
}

void DownloadManager::PauseAll(const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE, base::BindOnce(&DownloadManager::PauseAll,
                                  shared_from_this(), context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    return;
  }

  user_self_mgr->PauseAll(context_field);
}

void DownloadManager::Resume(const download::v1::ResumeRequest* request,
                             const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    auto copy_request = std::make_unique<download::v1::ResumeRequest>(*request);
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(&DownloadManager::Resume, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    return;
  }

  user_self_mgr->Resume(request, context_field);
}

void DownloadManager::ResumeAll(const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE, base::BindOnce(&DownloadManager::ResumeAll,
                                  shared_from_this(), context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    return;
  }

  user_self_mgr->ResumeAll(context_field);
}

void DownloadManager::Delete(const download::v1::DeleteRequest* request,
                             const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    auto copy_request = std::make_unique<download::v1::DeleteRequest>(*request);
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(&DownloadManager::Delete, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    return;
  }

  user_self_mgr->Delete(request, context_field);
}

void DownloadManager::DeleteCategory(
    const download::v1::DeleteCategoryRequest* request,
    const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    auto copy_request =
        std::make_unique<download::v1::DeleteCategoryRequest>(*request);
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(&DownloadManager::DeleteCategory, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    return;
  }

  user_self_mgr->DeleteCategory(request, context_field);
}

void DownloadManager::Retry(const download::v1::RetryRequest* request,
                            const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    auto copy_request = std::make_unique<download::v1::RetryRequest>(*request);
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(&DownloadManager::Retry, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    return;
  }

  user_self_mgr->Retry(request, context_field);
}

void DownloadManager::RetryAll(const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE, base::BindOnce(&DownloadManager::RetryAll,
                                  shared_from_this(), context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    return;
  }

  user_self_mgr->RetryAll(context_field);
}

void DownloadManager::Clean(const download::v1::CleanRequest* request,
                            const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    auto copy_request = std::make_unique<download::v1::CleanRequest>(*request);
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(&DownloadManager::Clean, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    return;
  }

  user_self_mgr->Clean(request, context_field);
}

void DownloadManager::CleanCategory(
    const download::v1::CleanCategoryRequest* request,
    const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    auto copy_request =
        std::make_unique<download::v1::CleanCategoryRequest>(*request);
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(&DownloadManager::CleanCategory, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    return;
  }
  user_self_mgr->CleanCategory(request, context_field);
}

void DownloadManager::Restore(const download::v1::RestoreRequest* request,
                              const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    auto copy_request =
        std::make_unique<download::v1::RestoreRequest>(*request);
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(&DownloadManager::Restore, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    return;
  }
  user_self_mgr->Restore(request, context_field);
}

void DownloadManager::RestoreAll(const ContextField& context_field) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE, base::BindOnce(&DownloadManager::RestoreAll,
                                  shared_from_this(), context_field));
    return;
  }

  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (!user_self_mgr) {
    return;
  }

  user_self_mgr->RestoreAll(context_field);
}

void DownloadManager::GetTaskInfoList(const ContextField& context_field,
                                      GetTaskListCallback callback) {
  if (!work_thread_->TaskRunner()->RunsTasksInCurrentSequence()) {
    work_thread_->TaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(&DownloadManager::GetTaskInfoList, shared_from_this(),
                       context_field, std::move(callback)));
    return;
  }

  std::string user_id = context_field.user_id();
  CreateDefaultDownloadDirectory(user_id);
  UserSelfMgrPtr user_self_mgr = FindUser(context_field);
  if (user_self_mgr) {
    user_self_mgr->GetTaskInfoList(context_field, std::move(callback));
  } else {
    std::vector<TaskInfoPtr> task_info_list;
    std::move(callback).Run(task_info_list);
  }
}

void DownloadManager::ResumeTasksFromDb() {
  std::map<std::string, UserInfoPtr> user_info_list;
  DownloadStoreHelper::GetInstance()->GetAllUsersInfo(user_info_list);
  for (const auto& [user_id, user_info] : user_info_list) {
    std::vector<TaskInfoPtr> task_info_list;
    DownloadStoreHelper::GetInstance()->GetTaskInfoList(user_id,
                                                        task_info_list);

    // 从数据库恢复数据时还不知道token是啥
    ContextField context_field("", user_id, "");
    UserSelfMgrPtr user_self_mgr = FindUser(context_field);
    if (!user_self_mgr) {
      user_self_mgr = AddUser(context_field);
    }
    user_self_mgr->SetUserInfo(user_info);
    user_self_mgr->ResumeFromDb(task_info_list, context_field);
  }
}

std::string DownloadManager::GetTorrentUploadPath(
    const ContextField& context_field) {
  base::FilePath dir_path = nas::utils::GetUploadTorrentFileCachePath();
  std::string torrent_path = path_unit::BaseFilePathToU8(dir_path);
  torrent_path = path_unit::UnifiedSlash(torrent_path);
  return torrent_path;
}

UserSelfMgrPtr DownloadManager::FindUser(const ContextField& context_field) {
  auto iter = user_self_mgr_list_.find(context_field.user_id());
  if (iter == user_self_mgr_list_.end()) {
    return nullptr;
  }
  iter->second->UpdateUserInfo(context_field);
  return iter->second;
}

UserSelfMgrPtr DownloadManager::AddUser(const ContextField& context_field) {
  scoped_refptr<base::SingleThreadTaskRunner> task_runner =
      AllocateTaskRunner(context_field);

  if (!task_runner) {
    LOG(ERROR) << "allocate task runner is null";
    return nullptr;
  }

  UserSelfMgrPtr user_self_mgr =
      std::make_shared<UserSelfMgr>(context_field, task_runner);
  user_self_mgr_list_[context_field.user_id()] = user_self_mgr;
  user_self_mgr->UpdateUserInfo(context_field);
  user_self_mgr->StartTimer();
  return user_self_mgr;
}

scoped_refptr<base::SingleThreadTaskRunner> DownloadManager::AllocateTaskRunner(
    const ContextField& context_field) {
  scoped_refptr<base::SingleThreadTaskRunner> task_runner = nullptr;
  // 目前最多只开20个线程，大于20个就用之前的线程
  if (user_thread_list_.size() >= 20) {
    int random_number = base::RandInt(0, 19);
    task_runner = user_thread_list_[random_number]->TaskRunner();
  } else {
    std::string name = kUserThreadName;
    name += "_";
    name += context_field.user_id();
    LOG(INFO) << "thread name is " << name;
    std::shared_ptr<NasThread> user_thread =
        std::make_shared<NasThread>(name.c_str(), base::MessagePumpType::IO);
    task_runner = user_thread->TaskRunner();
    user_thread_list_.push_back(user_thread);
  }

  return task_runner;
}

void DownloadManager::CreateDefaultDownloadDirectory(
    const std::string& user_id) {
  base::FilePath source_data = nas::GetUserSourceDataDir(user_id, true);
  base::FilePath user_default_download_path =
      source_data.Append(FILE_PATH_LITERAL("download"));
  if (!base::PathExists(user_default_download_path)) {
    base::File::Error err;
    if (!base::CreateDirectoryAndGetError(user_default_download_path, &err)) {
      LOG(ERROR) << "create dir failed: " << base::File::ErrorToString(err);
    }
  }
}

}  // namespace nas
