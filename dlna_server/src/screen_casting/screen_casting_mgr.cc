
// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/04 16:19
#include "screen_casting_mgr.h"

#include "base/guid.h"
#include "base/logging.h"
#include "base/strings/string_split.h"
#include "base/strings/stringprintf.h"
#include "url/gurl.h"

#include "nas/common/micro_service_info.hpp"

#include "utils.h"

namespace nas {

ScreenCastingMgr::ScreenCastingMgr(NasThread* work_thread,
                                   PLT_CtrlPointReference& ctrl_point)
    : work_thread_(work_thread), PLT_MediaController(ctrl_point) {
  PLT_MediaController::SetDelegate(this);
}

ScreenCastingMgr::~ScreenCastingMgr() {}

void ScreenCastingMgr::Init() {
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(base::BindOnce(
        &ScreenCastingMgr::Init, shared_from_this()));
    return;
  }
}

void ScreenCastingMgr::GetMediaRenderesList(
    ::dlna::v1::ListScreenCastingDevicesResponse* response,
    GetMediaRenderesListCallback callback) {
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(
        base::BindOnce(&ScreenCastingMgr::GetMediaRenderesList,
                       shared_from_this(), response, std::move(callback)));
    return;
  }

  const NPT_List<PLT_DeviceMapEntry*>& entries = media_renderers_.GetEntries();
  NPT_List<PLT_DeviceMapEntry*>::Iterator entry = entries.GetFirstItem();

  while (entry) {
    PLT_DeviceDataReference device = (*entry)->GetValue();
    auto info = response->add_devices();
    std::string name = device->GetFriendlyName().GetChars();
    std::string uuid = device->GetUUID().GetChars();
    std::string base_url = device->GetURLBase().ToString().GetChars();
    info->set_name(name);
    info->set_id(uuid);
    info->set_base_url(base_url);
    ++entry;
  }

  if (!callback.is_null()) {
    std::move(callback).Run();
  }
}

void ScreenCastingMgr::CreateTask(const std::string& device_id,
                                  const std::string& play_url,
                                  ContextField rpc_context_field,
                                  CreateTaskCallback callback) {
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(base::BindOnce(
        &ScreenCastingMgr::CreateTask, shared_from_this(), device_id, play_url,
        std::move(rpc_context_field), std::move(callback)));
    return;
  }

  DlnaServiceErrorNo err_code = DlnaServiceErrorNo::kSuccess;

  bool is_break = true;
  do {
    // 判断device 还在不在
    PLT_DeviceDataReference device_obj;
    FindRenderesDevice(device_id, device_obj);
    if (device_obj.IsNull()) {
      LOG(WARNING) << "device not exist: " << device_id;
      err_code = DlnaServiceErrorNo::kScreenCastingDeviceNotExist;
      break;
    }
#if 0
    // 先判断一下投屏设备是否支持该格式的文件
    err_code = DeviceSupportProtocol(device_id, play_url);
    if (err_code != DlnaServiceErrorNo::kSuccess) {
      break;
    }
#endif
    // 替换 host
    std::string new_play_url = play_url;
    err_code = ReplacePlayUrlHost(
        device_obj->GetLocalIP().ToString().GetChars(), new_play_url);
    if (err_code != DlnaServiceErrorNo::kSuccess) {
      break;
    }

    // 如果该设备已经存在我们自己的投屏任务的时，这时候重新设置播放链接，会触发其任务的check
    // 播放源的机制从而移除上一个任务
    std::string task_id = base::GenerateGUID();
    ScreenCastingTaskPtr task_ptr = std::make_shared<ScreenCastingTask>(
        task_id, new_play_url, device_obj, rpc_context_field);
    task_.insert(std::make_pair(task_id, task_ptr));
    task_ptr->Start(std::move(callback));  // 吧callback保存到task对象用，通过
                                           // 在 SetAVTransportURI 回调中调用
    // 给投屏设备设置播放链接,等待回调
    LOG(WARNING) << "SetAVTransportURI： " << new_play_url
                 << ", device: " << device_obj->GetFriendlyName().GetChars();
    SetAVTransportURI(device_obj, 0, new_play_url.c_str(), nullptr,
                      new std::string(task_id));
    is_break = false;

  } while (false);

  if (is_break && !callback.is_null()) {
    std::move(callback).Run("", err_code);
  }
}

void ScreenCastingMgr::CancelTask(const std::string& task_id) {
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(base::BindOnce(&ScreenCastingMgr::CancelTask,
                                          shared_from_this(), task_id));
    return;
  }

  PLT_DeviceDataReference device_obj;
  FindTaskRenderesDevice(task_id, device_obj);
  if (device_obj.IsNull()) {
    LOG(WARNING) << "device not exist";
    return;
  }
  Stop(device_obj, 0, new std::string(task_id));
}

void ScreenCastingMgr::CancelAllTask() {
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(
        base::BindOnce(&ScreenCastingMgr::CancelAllTask, shared_from_this()));
    return;
  }
  for (const auto& val : task_) {
    PLT_DeviceDataReference renderer_device = val.second->GetMediaRenderes();
    if (renderer_device.IsNull()) {
      LOG(WARNING) << "device not exist";
      continue;;
    }
    Stop(renderer_device, 0, new std::string(val.first));
  } 
}

void ScreenCastingMgr::ChangeTaskPlayUrl(const std::string& task_id,
                                         const std::string& play_url) {
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(base::BindOnce(&ScreenCastingMgr::ChangeTaskPlayUrl,
                                          shared_from_this(), task_id,
                                          play_url));
    return;
  }

  auto task_ptr = FindTask(task_id);
  if (!task_ptr) {
    LOG(WARNING) << "not find task, id: " << task_id;
    return;
  }

  PLT_DeviceDataReference device_obj = task_ptr->GetMediaRenderes();
  if (device_obj.IsNull()) {
    LOG(WARNING) << "device not exist";
    return;
  }

  // 替换 host
  std::string new_play_url = play_url;
  auto err_code = ReplacePlayUrlHost(
      device_obj->GetLocalIP().ToString().GetChars(), new_play_url);
  if (err_code != DlnaServiceErrorNo::kSuccess) {
    LOG(WARNING) << "ReplacePlayUrlHost failed: " << play_url;
    return;
  }
  task_ptr->ChangePlayUrl(play_url);
  SetAVTransportURI(device_obj, 0, play_url.c_str(), nullptr, nullptr);
}

void ScreenCastingMgr::PlayTask(const std::string& task_id) {
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(base::BindOnce(&ScreenCastingMgr::PlayTask,
                                          shared_from_this(), task_id));
    return;
  }

  PLT_DeviceDataReference device_obj;
  FindTaskRenderesDevice(task_id, device_obj);
  if (device_obj.IsNull()) {
    LOG(WARNING) << "device not exist";
    return;
  }
  Play(device_obj, 0, "1", nullptr);
}

void ScreenCastingMgr::PauseTask(const std::string& task_id) {
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(base::BindOnce(&ScreenCastingMgr::PauseTask,
                                          shared_from_this(), task_id));
    return;
  }
  PLT_DeviceDataReference device_obj;
  FindTaskRenderesDevice(task_id, device_obj);
  if (device_obj.IsNull()) {
    LOG(WARNING) << "device not exist";
    return;
  }
  Pause(device_obj, 0, nullptr);
}

void ScreenCastingMgr::SeekTask(const std::string& task_id,
                                const int64_t& position) {
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(base::BindOnce(&ScreenCastingMgr::SeekTask,
                                          shared_from_this(), task_id,
                                          std::move(position)));
    return;
  }
  PLT_DeviceDataReference device_obj;
  FindTaskRenderesDevice(task_id, device_obj);
  if (device_obj.IsNull()) {
    LOG(WARNING) << "device not exist";
    return;
  }
  std::string target = SecondsFormatTime(position);
  Seek(device_obj, 0, "REL_TIME", target.c_str(), NULL);
}

void ScreenCastingMgr::SetTaskVolume(const std::string& task_id, int volume) {
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(base::BindOnce(&ScreenCastingMgr::SetTaskVolume,
                                          shared_from_this(), task_id, volume));
    return;
  }
  PLT_DeviceDataReference device_obj;
  FindTaskRenderesDevice(task_id, device_obj);
  if (device_obj.IsNull()) {
    LOG(WARNING) << "device not exist";
    return;
  }
  SetVolume(device_obj, 0, "Master", volume, nullptr);
}

bool ScreenCastingMgr::OnMRAdded(PLT_DeviceDataReference& device) {
  // test if it's a media renderer
  PLT_Service* service;
  if (NPT_SUCCEEDED(device->FindServiceByType(
          "urn:schemas-upnp-org:service:AVTransport:*", service))) {
    auto name = device->GetFriendlyName();
    LOG(INFO) << "OnMRAdded media renderes: " << name.GetChars();
    GetProtocolInfo(device, nullptr);
    /*NPT_AutoLock lock(media_renderers_);
    auto name = device->GetFriendlyName();
    media_renderers_.Put(device->GetUUID(), device);
    LOG(INFO) << "added media renderes: " << name.GetChars()
              << ", host: " << device->GetURLBase().GetHost().GetChars();*/
  }

  return false;
}

void ScreenCastingMgr::OnMRRemoved(PLT_DeviceDataReference& device) {
  if (device.IsNull()) {
    LOG(WARNING) << "OnMRRemoved, device invalid";
    return;
  }

  NPT_String uuid = device->GetUUID();
  std::string name = device->GetFriendlyName().GetChars();
  {
    NPT_AutoLock lock(media_renderers_);
    if (media_renderers_.Erase(uuid) == NPT_SUCCESS) {
      LOG(INFO) << "removed media renderes: " << name;
    }
  }
}

void ScreenCastingMgr::OnMRStateVariablesChanged(
    PLT_Service* service,
    NPT_List<PLT_StateVariable*>* vars) {}

void ScreenCastingMgr::OnGetProtocolInfoResult(NPT_Result res,
                                               PLT_DeviceDataReference& device,
                                               PLT_StringList* sources,
                                               PLT_StringList* sinks,
                                               void* userdata) {
  if (NPT_FAILED(res)) {
    LOG(WARNING) << "OnGetProtocolInfoResult failed";
    // OnMRRemoved(device);
    return;
  }
  NPT_String uuid = device->GetUUID();
  std::string name = device->GetFriendlyName().GetChars();
  if (sinks->GetItemCount() < 1) {
    LOG(INFO) << "media renderes: " << name << ",none sinks";
    return;
  }
  NPT_AutoLock lock(media_renderers_);
  // 存在才添加
  PLT_DeviceDataReference* existing_device = nullptr;
  NPT_Result find_res = media_renderers_.Get(uuid, existing_device);
  if (find_res == NPT_ERROR_NO_SUCH_ITEM) {
    media_renderers_.Put(uuid, device);
    LOG(INFO) << "added media renderes: " << name
              << ", host: " << device->GetURLBase().GetHost().GetChars();

    work_thread_->PostTask(base::BindOnce(
        &ScreenCastingMgr::AddSinkProtocolInfo, shared_from_this(),
        std::string(uuid.GetChars()), *sinks));
  }
}

void ScreenCastingMgr::OnSetAVTransportURIResult(
    NPT_Result res,
    PLT_DeviceDataReference& device,
    void* userdata) {
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(
        base::BindOnce(&ScreenCastingMgr::OnSetAVTransportURIResult,
                       shared_from_this(), res, std::ref(device), userdata));
    return;
  }

  std::unique_ptr<std::string> task_id(static_cast<std::string*>(userdata));
  do {
    if (task_id == nullptr) {
      break;
    }
    auto task_obj = FindTask(*task_id);
    if (!task_obj) {
      LOG(WARNING) << "OnSetAVTransportURIResult, none task";
      break;
    }
    task_obj->OnStart(NPT_SUCCEEDED(res));
    if (NPT_FAILED(res)) {
      RemoveTask(task_obj->GetId());
      break;
    }
    work_thread_->TaskRunner()->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(&ScreenCastingMgr::StartTaskTimer, shared_from_this()),
        base::Seconds(1));

  } while (false);
}

void ScreenCastingMgr::OnStopResult(NPT_Result res,
                                    PLT_DeviceDataReference& device,
                                    void* userdata) {
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(base::BindOnce(&ScreenCastingMgr::OnStopResult,
                                          shared_from_this(), res,
                                          std::ref(device), userdata));
    return;
  }

  std::unique_ptr<std::string> task_id(static_cast<std::string*>(userdata));

  do {
    if (task_id == nullptr) {
      LOG(WARNING) << "OnStopResult, none task";
      break;
    }
    auto task_obj = FindTask(*task_id);
    if (!task_obj) {
      LOG(WARNING) << "OnStopResult, none task";
      break;
    }
    if (NPT_FAILED(res)) {
      LOG(WARNING) << "OnStopResult failed";
    }
    RemoveTask(task_obj->GetId());
  } while (false);
}

void ScreenCastingMgr::OnGetPositionInfoResult(NPT_Result res,
                                               PLT_DeviceDataReference& device,
                                               PLT_PositionInfo* info,
                                               void* userdata) {
  if (NPT_FAILED(res)) {
    LOG(WARNING) << "OnGetPositionInfoResult NPT_FAILED";
    return;
  }

  if (!work_thread_->IsCurrentThread()) {
    // 由于这里的 info
    // 参数是局部变量的地址传进来的，所以切换线程时需要好copy一份
    work_thread_->PostTask(base::BindOnce(
        &ScreenCastingMgr::OnGetPositionInfoResult, shared_from_this(), res,
        std::ref(device), new PLT_PositionInfo(*info), userdata));
    return;
  }

  std::unique_ptr<std::string> task_id(static_cast<std::string*>(userdata));
  std::unique_ptr<PLT_PositionInfo> position_info(info);

  do {
    if (task_id == nullptr) {
      LOG(WARNING) << "OnGetPositionInfoResult, none task";
      break;
    }
    auto task_obj = FindTask(*task_id);
    if (!task_obj) {
      LOG(WARNING) << "OnGetPositionInfoResult, none task";
      break;
    }
    task_obj->SetPositionInfo(*position_info);
  } while (false);
}

void ScreenCastingMgr::OnGetTransportInfoResult(NPT_Result res,
                                                PLT_DeviceDataReference& device,
                                                PLT_TransportInfo* info,
                                                void* userdata) {
  if (NPT_FAILED(res)) {
    LOG(WARNING) << "OnGetTransportInfoResult NPT_FAILED";
    return;
  }
  if (!work_thread_->IsCurrentThread()) {
    // 由于这里的 info
    // 参数是局部变量的地址传进来的，所以切换线程时需要好copy一份
    work_thread_->PostTask(base::BindOnce(
        &ScreenCastingMgr::OnGetTransportInfoResult, shared_from_this(), res,
        std::ref(device), new PLT_TransportInfo(*info), userdata));
    return;
  }

  std::unique_ptr<std::string> task_id(static_cast<std::string*>(userdata));
  std::unique_ptr<PLT_TransportInfo> transport_info(info);

  do {
    if (task_id == nullptr) {
      LOG(WARNING) << "OnGetTransportInfoResult, none task";
      break;
    }
    auto task_obj = FindTask(*task_id);
    if (!task_obj) {
      LOG(WARNING) << "OnGetTransportInfoResult, none task";
      break;
    }
    task_obj->SetTransportInfo(*transport_info);
  } while (false);
}

void ScreenCastingMgr::OnSetVolumeResult(NPT_Result res,
                                         PLT_DeviceDataReference& device,
                                         void* userdata) {
  if (NPT_FAILED(res)) {
    LOG(WARNING) << "OnSetVolumeResult NPT_FAILED";
    return;
  }
}

void ScreenCastingMgr::OnGetVolumeResult(NPT_Result res,
                                         PLT_DeviceDataReference& device,
                                         const char* channel,
                                         NPT_UInt32 volume,
                                         void* userdata) {
  if (NPT_FAILED(res)) {
    LOG(WARNING) << "OnGetVolumeResult NPT_FAILED";
    return;
  }
  if (!work_thread_->IsCurrentThread()) {
    // channel 暂时用不到
    work_thread_->PostTask(
        base::BindOnce(&ScreenCastingMgr::OnGetVolumeResult, shared_from_this(),
                       res, std::ref(device), nullptr, volume, userdata));
    return;
  }

  std::unique_ptr<std::string> task_id(static_cast<std::string*>(userdata));

  do {
    if (task_id == nullptr) {
      LOG(WARNING) << "OnGetTransportInfoResult, none task";
      break;
    }
    auto task_obj = FindTask(*task_id);
    if (!task_obj) {
      LOG(WARNING) << "OnGetTransportInfoResult, none task";
      break;
    }
    task_obj->SetVolume(volume);
  } while (false);
}

void ScreenCastingMgr::OnPauseResult(NPT_Result res,
                                     PLT_DeviceDataReference& device,
                                     void* userdata) {
  if (NPT_FAILED(res)) {
    LOG(WARNING) << "OnPauseResult NPT_FAILED";
    return;
  }
}

void ScreenCastingMgr::OnPlayResult(NPT_Result res,
                                    PLT_DeviceDataReference& device,
                                    void* userdata) {
  if (NPT_FAILED(res)) {
    LOG(WARNING) << "OnPlayResult NPT_FAILED";
    return;
  }
}

void ScreenCastingMgr::OnSeekResult(NPT_Result res,
                                    PLT_DeviceDataReference& device,
                                    void* userdata) {
  if (NPT_FAILED(res)) {
    LOG(WARNING) << "OnSeekResult NPT_FAILED";
    return;
  }
}

nas::ScreenCastingTaskPtr ScreenCastingMgr::FindTask(const std::string& id) {
  DCHECK(work_thread_->IsCurrentThread());

  nas::ScreenCastingTaskPtr task;

  auto itr = task_.find(id);
  if (itr != task_.end()) {
    task = itr->second;
  }
  return task;
}

void ScreenCastingMgr::FindTaskRenderesDevice(
    const std::string& task_id,
    PLT_DeviceDataReference& renderer_device) {
  auto task_ptr = FindTask(task_id);
  if (!task_ptr) {
    LOG(WARNING) << "not find task, id: " << task_id;
    return;
  }
  renderer_device = task_ptr->GetMediaRenderes();
}

void ScreenCastingMgr::FindRenderesDevice(
    const std::string& device_id,
    PLT_DeviceDataReference& renderer_device) {
  DCHECK(work_thread_->IsCurrentThread());
  PLT_DeviceDataReference* device_ptr = nullptr;
  media_renderers_.Get(device_id.c_str(), device_ptr);
  renderer_device =
      device_ptr ? *device_ptr
                 : PLT_DeviceDataReference();  // return empty reference if
                                               // not device was selected
}

void ScreenCastingMgr::RemoveTask(const std::string& id) {
  DCHECK(work_thread_->IsCurrentThread());
  task_.erase(id);
}

void ScreenCastingMgr::StartTaskTimer() {
  if (!check_task_ws_timer_.IsRunning()) {
    // 开始定时查询投屏页面是否关闭
    check_task_ws_timer_.Start(
        FROM_HERE, base::Seconds(30),
        base::BindRepeating(&ScreenCastingMgr::TimeingCheckPlayPageOnline,
                            shared_from_this()));
  }

  if (!query_timer_.IsRunning()) {
    // 开始定时查询任务的状态
    query_timer_.Start(
        FROM_HERE, base::Seconds(1),
        base::BindRepeating(&ScreenCastingMgr::TimeingQueryTaskStatus,
                            shared_from_this()));
  }
  if (!push_timer_.IsRunning()) {
    // 开始定时推送任务的状态
    push_timer_.Start(
        FROM_HERE, base::Seconds(1),
        base::BindRepeating(&ScreenCastingMgr::TimeingPushTaskStatus,
                            shared_from_this()));
  }
}

void ScreenCastingMgr::TimeingCheckPlayPageOnline() {
  DCHECK(work_thread_->IsCurrentThread());

  for (auto itr = task_.begin(); itr != task_.end(); itr++) {
    auto task = itr->second;

  
  }
}

void ScreenCastingMgr::ConnStausCallback(const std::string& id,
                                         const std::string& conn_count) {
  LOG(INFO) << "screencasting task ConnStausCallback conn_count:" << conn_count
            << ", id: " << id;
  if (!work_thread_->IsCurrentThread()) {
    work_thread_->PostTask(base::BindOnce(&ScreenCastingMgr::ConnStausCallback,
                                          shared_from_this(), id, conn_count));
    return;
  }

  int conn_count_value = 0;
  base::StringToInt(conn_count, &conn_count_value);

  auto itr = task_.find(id);
  if (itr == task_.end()) {
    return;
  }

  if (conn_count_value == 0) {
    LOG(INFO) << "screencasting task page ws closed.";
    task_.erase(itr);
  }

  if (task_.empty()) {
    check_task_ws_timer_.Stop();
    return;
  }
}

void ScreenCastingMgr::TimeingQueryTaskStatus() {
  DCHECK(work_thread_->IsCurrentThread());

  if (task_.empty()) {
    query_timer_.Stop();
    return;
  }

  for (auto itr = task_.begin(); itr != task_.end(); itr++) {
    // 查询状态
    QueryTaskStatusInfo(itr->second);
  }
}

void ScreenCastingMgr::TimeingPushTaskStatus() {
  DCHECK(work_thread_->IsCurrentThread());
  if (task_.empty()) {
    push_timer_.Stop();
    return;
  }

  for (auto itr = task_.begin(); itr != task_.end();) {
    auto task = itr->second;
    // 推送状态
    bool is_stop = task->NotifyStatusInfo();
    if (is_stop) {
      LOG(WARNING) << "media renderes state is STOPPED";
      itr = task_.erase(itr);
    } else {
      itr++;
    }
  }
}

void ScreenCastingMgr::QueryTaskStatusInfo(ScreenCastingTaskPtr task) {
  // 判断device 还在不在
  PLT_DeviceDataReference device_obj = task->GetMediaRenderes();
  if (device_obj.IsNull()) {
    LOG(WARNING) << "device not exist";
    return;
  }

  // 查询状态
  GetTransportInfo(device_obj, 0, new std::string(task->GetId()));

  // 查询 进度
  GetPositionInfo(device_obj, 0, new std::string(task->GetId()));

  // 查询音量
  GetVolume(device_obj, 0, "Master", new std::string(task->GetId()));
}

std::string ScreenCastingMgr::SecondsFormatTime(double seconds) {
  int hours = static_cast<int>(seconds) / 3600;
  int minutes = (static_cast<int>(seconds) % 3600) / 60;
  int remaining_seconds = static_cast<int>(seconds) % 60;

  char buffer[9];
  std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes,
                remaining_seconds);
  return std::string(buffer);
}

void ScreenCastingMgr::AddSinkProtocolInfo(const std::string& device_uuid,
                                           const PLT_StringList& sinks_info) {
  DCHECK(work_thread_->IsCurrentThread());
  std::vector<std::string> sinks_vec;
  for (PLT_StringList::Iterator it = sinks_info.GetFirstItem(); it; ++it) {
    std::vector<std::string> info_vec = base::SplitString(
        it->GetChars(), ":", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);

    if (info_vec.size() < 3) {
      LOG(WARNING) << "sinks protocol info format invaild";
      continue;
    }
    sinks_vec.push_back(info_vec.at(2));
  }
  sink_protocol_info_.insert(std::make_pair(device_uuid, sinks_vec));
}

DlnaServiceErrorNo ScreenCastingMgr::DeviceSupportProtocol(
    const std::string devcie_id,
    const std::string& play_url) {
  DlnaServiceErrorNo err_code = DlnaServiceErrorNo::kSuccess;
  // 先分离出url中的文件格式
  do {
    std::string ext = utils::GetFileExtensionFromUrl(play_url);
    if (ext.empty()) {
      LOG(WARNING) << "parse play url extension failed";
      err_code = DlnaServiceErrorNo::kPlayUrlInvalid;
      break;
    }

    // 然后根据文件格式找到对应的mime type
    auto mime_info = std::find_if(kMimeTypes.begin(), kMimeTypes.end(),
                                  [ext](const NginxMimeTypes& value) {
                                    for (const auto& val : value.file_extsion) {
                                      if (ext == val) {
                                        return true;
                                      }
                                    }
                                    return false;
                                  });

    if (mime_info == kMimeTypes.end()) {
      LOG(WARNING) << "nonsupport mime type, extension: " << ext;
      err_code = DlnaServiceErrorNo::kNonsupportMimeTypeExtension;
      break;
    }

    std::string mime_type = mime_info->mime_type;

    auto device_protocol = sink_protocol_info_.find(devcie_id);
    if (device_protocol == sink_protocol_info_.end()) {
      LOG(WARNING) << "device protocol not exist: " << devcie_id;
      err_code = DlnaServiceErrorNo::kScreenCastingDeviceNoneProtocol;
      break;
    }

    auto protocol_info = std::find(device_protocol->second.begin(),
                                   device_protocol->second.end(), mime_type);
    if (protocol_info == device_protocol->second.end()) {
      LOG(WARNING) << "device nonsupport mime type: " << mime_type
                   << ", device: " << devcie_id;
      err_code = DlnaServiceErrorNo::kScreenCastingDeviceNonsupportProtocol;
      break;
    }
  } while (false);

  return err_code;
}

DlnaServiceErrorNo ScreenCastingMgr::ReplacePlayUrlHost(
    const std::string& local_ip,
    std::string& play_url) {
  DlnaServiceErrorNo err_code = DlnaServiceErrorNo::kSuccess;
  do {
    GURL original_url(play_url);

    // 只针对我们自己的播放链接做host替换
    if (original_url.path().find("dynamic_res/v1/media/video/dlna/playfile") ==
        std::string::npos) {
      break;
    }

    // 获取当前nginx的port
    int nginx_port  = 0;
    std::string new_url = base::StringPrintf(
        "http://%s:%d%s",local_ip.c_str(),
        nginx_port, original_url.path().c_str());
    // 如果有查询参数，将其添加到新 URL 中
    if (!original_url.query().empty()) {
      new_url += "?" + original_url.query();
    }
    play_url = new_url;
  } while (false);

  return err_code;
}


}  // namespace nas
