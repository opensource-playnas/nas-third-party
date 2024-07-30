
// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/13 19:20

#ifndef NAS_MEDIA_SERVER_MICROMEDIACONTROLLER_SIMULATIONPLTMEDIARENDERER_H_
#define NAS_MEDIA_SERVER_MICROMEDIACONTROLLER_SIMULATIONPLTMEDIARENDERER_H_

#include "PltMediaRenderer.h"

#include "base/threading/thread.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
/*----------------------------------------------------------------------
|   PLT_MediaRenderer
+---------------------------------------------------------------------*/
class SimulationMediaRenderer : public PLT_MediaRenderer {
 public:
  SimulationMediaRenderer(const char* uuid);

  // PLT_DeviceHost methods
  virtual NPT_Result OnAction(PLT_ActionReference& action,
                              const PLT_HttpRequestContext& context);

 protected:
  virtual ~SimulationMediaRenderer();

 private:
  void SetTransportState(const char* state);
  void SetVol(PLT_ActionReference& action);
  void SetPlayUrl(PLT_ActionReference& action);
  void OnSeekTo(PLT_ActionReference& action);
  void UpdatePositionInfo(PLT_ActionReference& action);

  void StartTimer();
  void StopTimer();

  void PlayTimer();

  std::string SecondsFormatTime(double seconds);
  int TimeToSeconds(const std::string& formatted_time);

 private:
  PLT_Service* renderer_controller_service_ = nullptr;
  PLT_Service* av_transport_service_ = nullptr;


  std::atomic_int play_duration_ = 0;

  base::Thread play_thread_;
  base::RepeatingTimer play_timer_;
};

#endif  // NAS_MEDIA_SERVER_MICROMEDIACONTROLLER_SIMULATIONPLTMEDIARENDERER_H_