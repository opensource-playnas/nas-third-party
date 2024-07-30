// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/04 16:18

#ifndef NAS_DLNA_SERVER_SRC_SCREEN_CASTING_SCREEN_CASTING_TASK_H_
#define NAS_DLNA_SERVER_SRC_SCREEN_CASTING_SCREEN_CASTING_TASK_H_

#include <list>
#include <memory>
#include <string>

#include "base/callback.h"
#include <base/files/file_path.h>
#include "nas/dlna_server/protos/dlna_server.grpc.pb.h"

#include "Platinum.h"
#include "PltMediaRenderer.h"
#include "PltVersion.h"

#include "nas/common/context_field.h"

#include "public_define.h"


namespace nas {

using CreateTaskCallback =
    base::OnceCallback<void(const std::string&, DlnaServiceErrorNo)>;

class ScreenCastingTask
    : public std::enable_shared_from_this<ScreenCastingTask> {
  enum PlayStatus {
    kUnknown = 0,
    kPLAYING,
    kPAUSE,
    kSTOPED,
  };

 public:
  explicit ScreenCastingTask(const std::string& id,
                             const std::string& play_url,
                             PLT_DeviceDataReference media_renderes,
                             ContextField& rpc_context_field);
  ~ScreenCastingTask();
  std::string GetId() { return id_; }
  PLT_DeviceDataReference GetMediaRenderes() { return media_renderes_; }
  void Start(CreateTaskCallback callback);
  void OnStart(bool succ);

  bool NotifyStatusInfo();

  bool IsStoped() { return stoped_count_ > 5; }

  const ContextField& GetContextField() { return rpc_context_field_; }

  void SetPositionInfo(const PLT_PositionInfo& position_info) {
    position_info_ = position_info;
  }

  void SetTransportInfo(const PLT_TransportInfo& transport_info) {
    transport_info_ = transport_info;
  }

  void SetVolume(NPT_UInt32 volume) { volume_ = volume; }

  void ChangePlayUrl(const std::string& play_url) { play_url_ = play_url; }

 private:
  PlayStatus GetTaskPlayStatus();

  bool ChangedPlayUrl();

 private:
  CreateTaskCallback start_callback_;
  std::string id_;
  std::string play_url_;
  /* Currently selected media renderer as well as
   * a lock.  If you ever want to hold both the m_CurMediaRendererLock lock and
   * the m_CurMediaServerLock lock, make sure you grab the server lock first.
   */
  PLT_DeviceDataReference media_renderes_;
  NPT_Mutex media_renderes_lock_;

  ContextField rpc_context_field_;
  PLT_PositionInfo position_info_;
  PLT_TransportInfo transport_info_;
  NPT_UInt32 volume_;
  int stoped_count_ = 0;
  std::string last_notify_content_;
};

using ScreenCastingTaskPtr = std::shared_ptr<ScreenCastingTask>;
}  // namespace nas

#endif  // NAS_DLNA_SERVER_SRC_SCREEN_CASTING_SCREEN_CASTING_TASK_H_
