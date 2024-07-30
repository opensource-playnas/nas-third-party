// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/04 16:19

#include "screen_casting_task.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "url/gurl.h"

namespace nas {

ScreenCastingTask::ScreenCastingTask(const std::string& id,
                                     const std::string& play_url,
                                     PLT_DeviceDataReference media_renderes,
                                     ContextField& rpc_context_field)
    : id_(id), play_url_(play_url), rpc_context_field_(rpc_context_field) {
  NPT_AutoLock lock(media_renderes_lock_);
  media_renderes_ = media_renderes;
  rpc_context_field_.set_call_id(id_);
}

ScreenCastingTask::~ScreenCastingTask() {}

void ScreenCastingTask::Start(CreateTaskCallback callback) {
  start_callback_ = std::move(callback);
}

void ScreenCastingTask::OnStart(bool succ) {
  if (start_callback_.is_null()) {
    LOG(WARNING) << "start callback is null";
    return;
  }
  DlnaServiceErrorNo errcode = DlnaServiceErrorNo::kSuccess;
  std::string ret_id = id_;
  if (!succ) {
    ret_id.clear();
    errcode = DlnaServiceErrorNo::kCreateScreenCastingFailed;
  }
  std::move(start_callback_).Run(ret_id, errcode);
  start_callback_.Reset();
}

bool ScreenCastingTask::NotifyStatusInfo() {
  auto status = GetTaskPlayStatus();

  if (status == kSTOPED) {
    if (stoped_count_ < 3) {
      status = kUnknown;
    }
    ++stoped_count_;
  }
  // 改变了播放链接以后，当前任务就认为是停止了
  bool changed = ChangedPlayUrl();
  if (changed) {
    LOG(WARNING) << "device play url changed, before: " << play_url_
                 << ", after: " << position_info_.track_uri.GetChars()
                 << ", id: " << id_;
    status = kSTOPED;
  }

  base::Value json_data(base::Value::Type::DICT);
  json_data.SetIntKey("status", (int)status);
  json_data.SetStringKey(
      "duration",
      base::NumberToString(position_info_.track_duration.ToSeconds()));
  json_data.SetStringKey(
      "rel_time", base::NumberToString(position_info_.rel_time.ToSeconds()));
  json_data.SetIntKey("volume", volume_);
  json_data.SetStringKey("media_url", position_info_.track_uri.GetChars());

  base::Value json_obj(base::Value::Type::DICT);
  json_obj.SetIntKey("errno", 0);
  json_obj.SetStringKey("msg", "");
  json_obj.SetKey("data", std::move(json_data));

  std::string content_json;
  base::JSONWriter::Write(json_obj, &content_json);

  if (last_notify_content_ == content_json && status != kSTOPED) {
    LOG(INFO) << "already notify";
    return false;
  }
  last_notify_content_ = content_json;

  return changed ? true : status == kSTOPED;
}

nas::ScreenCastingTask::PlayStatus ScreenCastingTask::GetTaskPlayStatus() {
  auto ret = PlayStatus::kUnknown;
  do {
    if (transport_info_.cur_transport_status != "OK") {
      break;
    }

    if (transport_info_.cur_transport_state == "PLAYING") {
      ret = PlayStatus::kPLAYING;
    } else if (transport_info_.cur_transport_state == "PAUSED_PLAYBACK") {
      ret = PlayStatus::kPAUSE;
    } else if (transport_info_.cur_transport_state == "STOPPED") {
      ret = PlayStatus::kSTOPED;
    } else if (transport_info_.cur_transport_state == "NO_MEDIA_PRESENT") {
      ret = PlayStatus::kSTOPED;
    }

  } while (false);

  return ret;
}

bool ScreenCastingTask::ChangedPlayUrl() {
  bool changed = false;

  do {
    if (position_info_.track_uri.IsEmpty()) {
      break;
    }
    std::string cur_play_url = position_info_.track_uri.GetChars();

    GURL play_url(play_url_);
    GURL track_url(cur_play_url);

    changed = play_url.path() != track_url.path() ||
              play_url.query() != track_url.query();

  } while (false);

  return changed;
}

}  // namespace nas
