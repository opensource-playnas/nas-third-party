// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/04 16:18

#ifndef NAS_DLNA_SERVER_SRC_SCREEN_CASTING_SCREEN_CASTING_MGR_H_
#define NAS_DLNA_SERVER_SRC_SCREEN_CASTING_SCREEN_CASTING_MGR_H_

#include <map>
#include <memory>
#include <string>
#include "base/files/file_util.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread.h"
#include "base/time/time.h"
#include "base/timer/timer.h"

#include "Platinum.h"
#include "PltMediaRenderer.h"
#include "PltVersion.h"

#include "nas/common/nas_thread.h"
#include "public_define.h"

#include "screen_casting/screen_casting_task.h"

namespace nas {

using GetMediaRenderesListCallback = base::OnceCallback<void()>;

class ScreenCastingMgr : public PLT_MediaController,
                         public PLT_MediaControllerDelegate,
                         public std::enable_shared_from_this<ScreenCastingMgr> {
 public:
  explicit ScreenCastingMgr(NasThread* work_thread,
                            PLT_CtrlPointReference& ctrl_point);
  ~ScreenCastingMgr();

  void Init();

  void GetMediaRenderesList(
      ::dlna::v1::ListScreenCastingDevicesResponse* response,
      GetMediaRenderesListCallback callback);

  void CreateTask(const std::string& device_id,
                  const std::string& play_url,
                  ContextField rpc_context_field,
                  CreateTaskCallback callback);

  void CancelTask(const std::string& task_id);
  void CancelAllTask();
  void ChangeTaskPlayUrl(const std::string& task_id,
                         const std::string& play_url);
  void PlayTask(const std::string& task_id);
  void PauseTask(const std::string& task_id);
  void SeekTask(const std::string& task_id, const int64_t & position);
  void SetTaskVolume(const std::string& task_id, int volume);

 protected:
  // PLT_MediaControllerDelegate methods
  bool OnMRAdded(PLT_DeviceDataReference& device);
  void OnMRRemoved(PLT_DeviceDataReference& device);
  void OnMRStateVariablesChanged(PLT_Service* service,
                                 NPT_List<PLT_StateVariable*>* vars);

  virtual void OnGetProtocolInfoResult(NPT_Result res,
                                       PLT_DeviceDataReference& device,
                                       PLT_StringList* sources,
                                       PLT_StringList* sinks,
                                       void* userdata);

  virtual void OnSetAVTransportURIResult(NPT_Result res,
                                         PLT_DeviceDataReference& device,
                                         void* userdata);

  virtual void OnStopResult(NPT_Result res,
                            PLT_DeviceDataReference& device,
                            void* userdata);

  void OnGetPositionInfoResult(NPT_Result res,
                               PLT_DeviceDataReference& device,
                               PLT_PositionInfo* info,
                               void* userdata);
  virtual void OnGetTransportInfoResult(NPT_Result res,
                                        PLT_DeviceDataReference& device,
                                        PLT_TransportInfo* info,
                                        void* userdata);

  virtual void OnSetVolumeResult(NPT_Result res,
                                 PLT_DeviceDataReference& device,
                                 void* userdata);
  virtual void OnGetVolumeResult(NPT_Result res,
                                 PLT_DeviceDataReference& device,
                                 const char* channel,
                                 NPT_UInt32 volume,
                                 void* userdata);
  virtual void OnPauseResult(NPT_Result res,
                             PLT_DeviceDataReference& device,
                             void* userdata);

  virtual void OnPlayResult(NPT_Result res,
                            PLT_DeviceDataReference& device,
                            void* userdata);

  virtual void OnSeekResult(NPT_Result res,
                            PLT_DeviceDataReference& device,
                            void* userdata);

 private:
  ScreenCastingTaskPtr FindTask(const std::string& id);

  void FindTaskRenderesDevice(const std::string& task_id,
                              PLT_DeviceDataReference& renderer_device);

  void FindRenderesDevice(const std::string& device_id,
                          PLT_DeviceDataReference& renderer_device);
  void RemoveTask(const std::string& id);

  void StartTaskTimer();

  // 定时检查页面长连接是正常
  void TimeingCheckPlayPageOnline();

  void ConnStausCallback(const std::string& id, const std::string& conn_count);

  // 定时查询任务状态
  void TimeingQueryTaskStatus();
  // 定时推送任务状态
  void TimeingPushTaskStatus();

  void QueryTaskStatusInfo(ScreenCastingTaskPtr task);

  std::string SecondsFormatTime(double seconds);

  void AddSinkProtocolInfo(const std::string& device_uuid,
                           const PLT_StringList& sinks_info);

  DlnaServiceErrorNo DeviceSupportProtocol(const std::string devcie_id,
                                           const std::string& play_url);
  DlnaServiceErrorNo ReplacePlayUrlHost(const std::string& local_ip,
                                        std::string& play_url);

 private:
  std::map<std::string, ScreenCastingTaskPtr> task_;
  std::map<std::string, std::vector<std::string>> sink_protocol_info_;
  NasThread* work_thread_ = nullptr;
  base::RepeatingTimer check_task_ws_timer_;
  base::RepeatingTimer query_timer_;
  base::RepeatingTimer push_timer_;

  /* Tables of known devices on the network.  These are updated via the
   * OnMSAddedRemoved and OnMRAddedRemoved callbacks.  Note that you should
   * first lock before accessing them using the NPT_Map::Lock function.
   */
  NPT_Lock<PLT_DeviceMap> media_renderers_;  //  投屏设备
};

}  // namespace nas

#endif  // NAS_DLNA_SERVER_SRC_SCREEN_CASTING_SCREEN_CASTING_MGR_H_
