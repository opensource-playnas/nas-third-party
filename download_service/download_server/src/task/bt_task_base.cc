#include "bt_task_base.h"

#include "base/bind.h"
#include "base/guid.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/stringprintf.h"
#include "base/task/thread_pool.h"
#include "base/threading/thread_task_runner_handle.h"

#include "nas/common/path/file_path_unit.h"
#include "utils/utils.h"

namespace nas {

BtTaskBase::BtTaskBase(TaskParamPtr param, const ContextField& context_field)
    : TaskBase(param, context_field) {
  task_impl_ = GetBtDownloadTaskImpl(param->type);
  if (!IsResumeFromDb(param) && task_impl_) {
    ModifyTorrentPath(param);
    TorrentTaskParamPtr torrent_param =
        std::static_pointer_cast<TorrentTaskParam>(param);
    task_info_->SetTaskParam(torrent_param);
    SetDownloadParam();
  }
}

BtTaskBase::~BtTaskBase() {
  if (task_impl_) {
    DownloadInterfaceHelper::GetInstance()->ReleaseTorrentTask(task_impl_);
    task_impl_ = NULL;
  }

  if (item_select_list_) {
    delete[] item_select_list_;
    item_select_list_ = NULL;
  }
}

bool BtTaskBase::Parse() {
  return true;
}

DownloadErrorCode BtTaskBase::Start() {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  if (task_info_ && task_info_->IsStarted()) {
    return DownloadErrorCode::kNormal;
  }
  SetDownloadParam();
  task_impl_->SetTorrentAddedCallback(this, &BtTaskBase::OnTorrentAdded);
  DownloadErrorCode ret = task_impl_->Start();
  if (ret == DownloadErrorCode::kNormal) {
    task_info_->SetIsStarted(true);
    task_info_->SetIsValid(true);
  }
  return err;
}

DownloadErrorCode BtTaskBase::CheckCondition() {
  if (!task_impl_) {
    return DownloadErrorCode::kCreateTaskFailed;
  }
  return task_impl_->CheckCondition(GetTaskParam()->url.c_str());
}

void BtTaskBase::UpdateProgressInfo(base::Value::Dict* result) {
  TorrentDownloadProgressInfo download_progress_info;
  TaskParamPtr task_param = task_info_->GetTaskParam();
  TaskBaseInfoPtr task_base_info = task_info_->GetTaskBaseInfo();
  TaskProgressInfoPtr progress_info = task_info_->GetProgressInfo();
  download_progress_info.total_progress = progress_info->GetTotalProgress();
  TorrentTaskParamPtr torrent_param =
      std::static_pointer_cast<TorrentTaskParam>(task_param);
  if (!torrent_param) {
    return;
  }

  task_impl_->GetProgressInfo(&download_progress_info);
  progress_info->SetTotalProgress(download_progress_info.total_progress);
  progress_info->SetDownloadSpeed(download_progress_info.download_speed);
  progress_info->SetUploadSpeed(download_progress_info.upload_speed);
  progress_info->SetErrorCode(download_progress_info.err_code);

  if (task_base_info->GetFileList().empty()) {
    base::ThreadPool::PostTaskAndReplyWithResult(
        FROM_HERE, {base::MayBlock()},
        base::BindOnce(
            &BtTaskBase::GetTorrentList,
            std::static_pointer_cast<BtTaskBase>(shared_from_this())),
        base::BindOnce(
            &BtTaskBase::SetFileList,
            std::static_pointer_cast<BtTaskBase>(shared_from_this())));
  }

  if (IsDownloadFinished() && GetTaskStatus() != TaskStatus::kSeeding) {
    SetTaskStatus(TaskStatus::kFinished);
    PublishFinishedTaskInfo();
    SendTaskEndMessage(
        true, task_info_->GetTaskBaseInfo()->GetTaskAttribute().file_name);
  }

  if (task_impl_->IsChecking()) {
     SetTaskStatus(TaskStatus::kChecking);
  } else if (task_impl_->IsDownloading()) {
    SetTaskStatus(TaskStatus::kDownloading);
  }

  if (download_progress_info.is_seed ||
      GetTaskStatus() == TaskStatus::kFinished) {
    SetTaskStatus(TaskStatus::kSeeding);
  }

  *result = BuildDownloadingData();
}

void BtTaskBase::UpdateSeedingInfo(base::Value::Dict* result) {
  DCHECK(task_impl_);

  char* info = task_impl_->GetSeedingInfo();
  if (!info) {
    LOG(WARNING) << "seeding info is NULL";
    return;
  }
  absl::optional<base::Value> json_value = base::JSONReader::Read(info);
  if (!json_value) {
    LOG(WARNING) << "read seeding info json failed";
    return;
  }

  if (base::Value::Dict* dict = json_value->GetIfDict()) {
    *result = std::move(*dict);
  }
  ReleaseBuf(info);
}

bool BtTaskBase::CheckSeedingLimits() {
  bool reach_limit = false;
  bool is_share_ratio_limit =
      SystemCloudConfig::GetInstance()->IsShareRatioLimit();
  bool is_seeding_time_limit =
      SystemCloudConfig::GetInstance()->IsSeedingTimeLimit();
  if (is_share_ratio_limit) {
    // 将种子的分享率与限制值比较
    int real_share_ratio = task_impl_->GetTorrentShareRatio();
    int share_ratio_limit_value =
        SystemCloudConfig::GetInstance()->GetShareRatioLimitValue();
    if (real_share_ratio >= share_ratio_limit_value) {
      LOG(INFO) << "reach share_ratio limit, share_ratio = "
                << real_share_ratio;
      reach_limit = true;
    }
  }

  if (is_seeding_time_limit) {
    // 将种子的做种时间与限制值比较
    int64_t seeding_time = task_impl_->GetSeedingTime();
    int seeding_time_limit_value =
        SystemCloudConfig::GetInstance()->GetSeedingTimeLimitValue();
    if (seeding_time >= seeding_time_limit_value * 60) {
      LOG(INFO) << "reach seeding time limit, seeding time = " << seeding_time;
      reach_limit = true;
    }
  }

  if (reach_limit) {
    LOG(INFO) << "reach seeding limit";
    if (task_impl_->Pause()) {
      LOG(INFO) << "reach seeding limit, set task finished";
      SetTaskStatus(TaskStatus::kFinished);
      PublishFinishedTaskInfo();
    }
  }
  return reach_limit;
}

int BtTaskBase::GetUploadRate() {
  return task_impl_->GetUploadRate();
}

DownloadTaskInterface* BtTaskBase::GetDownloadTaskImp() {
  DCHECK(task_impl_);
  return task_impl_;
}

void BtTaskBase::SetDownloadParam() {
  TaskParamPtr param = task_info_->GetTaskParam();
  TorrentTaskParamPtr task_param =
      std::static_pointer_cast<TorrentTaskParam>(param);

  DCHECK(task_param);
  if (!task_param) {
    return;
  }

  TorrentDownloadParam download_param;
  memcpy(download_param.id, task_param->id.c_str(), task_param->id.size());
  memcpy(download_param.url, task_param->url.c_str(), task_param->url.size());
  memcpy(download_param.save_path, task_param->save_path.c_str(),
         task_param->save_path.size());
  memcpy(download_param.file_name, task_param->file_name.c_str(),
         task_param->file_name.size());
  memcpy(download_param.file_name, task_param->file_name.c_str(),
         task_param->file_name.size());
  download_param.is_download_now = task_param->is_download_now;

  do {
    const int size = task_param->torrent_item_select_list.size();
    if (size <= 0) {
      break;
    }
    LOG(INFO) << "select item size: " << size;
    item_select_list_ = new TorrenFileItemSelect[size];
    for (int i = 0; i < size; ++i) {
      item_select_list_[i] = task_param->torrent_item_select_list[i];
    }
    download_param.item_select_list = item_select_list_;
    download_param.file_count = size;

  } while (0);

  task_impl_->SetParam(download_param);
  param->hash = GetTaskHash();
}

void BtTaskBase::ResumeDataFromDb(TaskInfoPtr task_info) {
  DCHECK(task_info);
  task_info_ = task_info;
  SetDownloadParam();
  ParseTorrent();
}

int BtTaskBase::SyncParse(ParseResult* result) {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  do {
    TaskParamPtr param = task_info_->GetTaskParam();
    if (!task_impl_->IsValid(param->url.c_str())) {
      if (param->type == DownloadType::kTorrent) {
        err = DownloadErrorCode::kTorrentFileInvalid;
      } else {
        err = DownloadErrorCode::kMagnetLinkInvalid;
      }
      break;
    }

    ParseTorrent();
  } while (0);

  return err;
}

bool BtTaskBase::ParseTorrent() {
  TaskBaseInfoPtr task_base_info = task_info_->GetTaskBaseInfo();
  FileBaseInfo base_info = task_impl_->GetTorrentFileBaseInfo();
  task_base_info->SetTaskName(base_info.file_name);
  task_base_info->SetTaskFileSize(base_info.file_size);
  task_base_info->SetTaskFileType(
      std::to_string(nas::path_unit::FileType::kBt));
  task_base_info->SetTaskLandFileName(base_info.file_name);

  CreateMagnetUri();
  const char* torrent_list = GetTorrentList();
  if (!torrent_list) {
    return false;
  }
  SetFileList(torrent_list);
  return true;
}

BtDownloadTask* BtTaskBase::GetBtDownloadTaskImpl(DownloadType type) {
  BtDownloadTask* task = nullptr;
  if (type == DownloadType::kTorrent) {
    task =
        DownloadInterfaceHelper::GetInstance()->CreateTorrentFileDownloadTask();
  } else if (type == DownloadType::kMagnet) {
    task = DownloadInterfaceHelper::GetInstance()->CreateMagnetDownloadTask();
  }
  return task;
}

void BtTaskBase::ReleaseBuf(const char* buf) {
  if (task_impl_ && buf) {
    task_impl_->ReleaseBuf(buf);
  }
}

void BtTaskBase::SetFileList(const char* torrent_list) {
  if (torrent_list) {
    TaskBaseInfoPtr task_base_info = task_info_->GetTaskBaseInfo();
    task_base_info->SetFileList(torrent_list);
    ReleaseBuf(torrent_list);
  }
}

const char* BtTaskBase::GetTorrentList() {
  return task_impl_->GetTorrentList();
}

void BtTaskBase::ModifyTorrentPath(TaskParamPtr param) {
  TorrentTaskParamPtr torrent_param =
      std::static_pointer_cast<TorrentTaskParam>(param);
  // 复制种子文件到data下面去，防止种子文件被删除，重启服务后找不到种子文件路径问题
  if (param->type == DownloadType::kTorrent && torrent_param &&
      !torrent_param->is_parse_task) {
    base::FilePath src_path = nas::path_unit::BaseFilePathFromU8(param->url);
    base::FilePath dest_path = nas::utils::GetTorrentFileSavePath();

    std::string torrent_file = base::StringPrintf(
        "%s.torrent",
        base::GUID::GenerateRandomV4().AsLowercaseString().c_str());
    dest_path =
        dest_path.Append(nas::path_unit::BaseFilePathFromU8(torrent_file));

    bool result = base::CopyFile(src_path, dest_path);
    if (result) {
      param->url = nas::path_unit::BaseFilePathToU8(dest_path);
    } else {
      LOG(WARNING) << "copy torrent file failed! path: " << src_path;
    }
  }
}

bool BtTaskBase::Resume() {
  TaskStatus status = GetTaskStatus();
  if (status == TaskStatus::kWaitting &&
      !SystemCloudConfig::GetInstance()->IsBtAutoDownload()) {
    SetTaskStatus(TaskStatus::kPaused);
    return true;
  }

  bool is_failed = (status == TaskStatus::kFailed) || (!task_info_->IsValid());
  if (GetDownloadTaskImp()->Resume(is_failed)) {
    SetTaskStatus(TaskStatus::kDownloading);
    task_info_->SetIsValid(true);
    return true;
  }

  return false;
}

void BtTaskBase::OnTorrentAdded(void* context) {
  LOG(INFO) << "torrent add completed!";
  BtTaskBase* task = (BtTaskBase*)context;
  if (!task) {
    return;
  }

  task->UpdateAndPublishTorrentInfo();
}

void BtTaskBase::UpdateAndPublishTorrentInfo() {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&BtTaskBase::UpdateAndPublishTorrentInfo,
                                  base::Unretained(this)));
    return;
  }
  ParseTorrent();
  PublishTaskBaseInfo();
}

}  // namespace nas
