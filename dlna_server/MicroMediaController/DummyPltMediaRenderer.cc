

// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/13 19:23

#include "DummyPltMediaRenderer.h"

#include <iostream>
#include <sstream>

#include "Neptune.h"
#include "PltService.h"

SimulationMediaRenderer::SimulationMediaRenderer(const char* uuid)
    : play_thread_("play_thread"),
      PLT_MediaRenderer("Nas Test Media Renderer", false, uuid) {
  play_thread_.Start();
}

NPT_Result SimulationMediaRenderer::OnAction(
    PLT_ActionReference& action,
    const PLT_HttpRequestContext& context) {
  NPT_COMPILER_UNUSED(context);

  // 为了测试 只接受本地ip 的请求
  if (context.GetLocalAddress().GetIpAddress().AsLong() !=
      context.GetRemoteAddress().GetIpAddress().AsLong()) {
    return NPT_FAILURE;
  }

  /* parse the action name */
  NPT_String name = action->GetActionDesc().GetName();

  // since all actions take an instance ID and we only support 1 instance
  // verify that the Instance ID is 0 and return an error here now if not
  NPT_String serviceType =
      action->GetActionDesc().GetService()->GetServiceType();
  if (serviceType.Compare("urn:schemas-upnp-org:service:AVTransport:1", true) ==
      0) {
    if (NPT_FAILED(action->VerifyArgumentValue("InstanceID", "0"))) {
      action->SetError(718, "Not valid InstanceID");
      return NPT_FAILURE;
    }
  }
  serviceType = action->GetActionDesc().GetService()->GetServiceType();
  if (serviceType.Compare("urn:schemas-upnp-org:service:RenderingControl:1",
                          true) == 0) {
    if (NPT_FAILED(action->VerifyArgumentValue("InstanceID", "0"))) {
      action->SetError(702, "Not valid InstanceID");
      return NPT_FAILURE;
    }
  }

  /* Is it a ConnectionManager Service Action ? */
  if (name.Compare("GetCurrentConnectionInfo", true) == 0) {
    return OnGetCurrentConnectionInfo(action);
  }

  do {
    /* Is it a AVTransport Service Action ? */
    if (name.Compare("Pause", true) == 0) {
      SetTransportState("PAUSED_PLAYBACK");
      break;
    }
    if (name.Compare("Play", true) == 0) {
      SetTransportState("PLAYING");
      break;
    }
    if (name.Compare("Seek", true) == 0) {
      OnSeekTo(action);
      break;
    }
    if (name.Compare("Stop", true) == 0) {
      SetTransportState("STOPPED");
      break;
    }
    if (name.Compare("SetAVTransportURI", true) == 0) {
      SetPlayUrl(action);
      break;
    }
    if (name.Compare("SetVolume", true) == 0) {
      SetVol(action);
      break;
    }
    if (name.Compare("GetPositionInfo", true) == 0) {
      UpdatePositionInfo(action);
    }
    // other actions rely on state variables
    action->SetArgumentsOutFromStateVariable();
  } while (false);

  return NPT_SUCCESS;
}

SimulationMediaRenderer::~SimulationMediaRenderer() {}

void SimulationMediaRenderer::SetTransportState(const char* state) {
  if (!av_transport_service_) {
    FindServiceById("urn:upnp-org:serviceId:AVTransport",
                    av_transport_service_);
  }
  av_transport_service_->SetStateVariable("TransportState", state);
  av_transport_service_->SetStateVariable("TransportStatus", "OK");
  av_transport_service_->SetStateVariable("TransportPlaySpeed", "1");

  NPT_String state_str = state;

  if (state_str.Compare("PLAYING", true) == 0) {
    // 开始计时
    play_thread_.task_runner()->PostTask(
        FROM_HERE, base::BindRepeating(&SimulationMediaRenderer::StartTimer,
                                       base::Unretained(this)));

  } else if (state_str.Compare("PAUSED_PLAYBACK", true) == 0) {
    // 暂停计时
    play_thread_.task_runner()->PostTask(
        FROM_HERE, base::BindRepeating(&SimulationMediaRenderer::StopTimer,
                                       base::Unretained(this)));

  } else if (state_str.Compare("STOPPED", true) == 0) {
    // 删除计时
    play_thread_.task_runner()->PostTask(
        FROM_HERE, base::BindRepeating(&SimulationMediaRenderer::StopTimer,
                                       base::Unretained(this)));
    play_duration_ = 0;
    av_transport_service_->SetStateVariable("AVTransportURI", "");
    av_transport_service_->SetStateVariable("CurrentTrackDuration", "00:00:00");
    av_transport_service_->SetStateVariable("CurrentTrackURI", "");
  }
}

void SimulationMediaRenderer::SetVol(PLT_ActionReference& action) {
  if (!renderer_controller_service_) {
    FindServiceById("urn:upnp-org:serviceId:RenderingControl",
                    renderer_controller_service_);
  }
  NPT_String volume;
  action->GetArgumentValue("DesiredVolume", volume);
  renderer_controller_service_->SetStateVariable("Volume", volume);
  renderer_controller_service_->SetStateVariableExtraAttribute(
      "Volume", "Channel", "Master");
}

void SimulationMediaRenderer::SetPlayUrl(PLT_ActionReference& action) {
  // default implementation is using state variable
  NPT_String uri;
  action->GetArgumentValue("CurrentURI", uri);

  NPT_String metadata;
  action->GetArgumentValue("CurrentURIMetaData", metadata);
  if (!av_transport_service_) {
    FindServiceById("urn:upnp-org:serviceId:AVTransport",
                    av_transport_service_);
  }
  // update service state variables
  av_transport_service_->SetStateVariable("AVTransportURI", uri);
  av_transport_service_->SetStateVariable("AVTransportURIMetaData", metadata);
  av_transport_service_->SetStateVariable("CurrentTrackDuration", "01:00:00");
  av_transport_service_->SetStateVariable("CurrentTrackMetadata", metadata);
  av_transport_service_->SetStateVariable("CurrentTrackURI", uri);
  play_duration_ = 0;
  SetTransportState("PLAYING");
}

void SimulationMediaRenderer::OnSeekTo(PLT_ActionReference& action) {
  // default implementation is using state variable
  NPT_String target_position;
  action->GetArgumentValue("Target", target_position);
  play_duration_ = TimeToSeconds(target_position.GetChars());
}

void SimulationMediaRenderer::UpdatePositionInfo(PLT_ActionReference& action) {
  // 更新进度到变量中
  if (!av_transport_service_) {
    FindServiceById("urn:upnp-org:serviceId:AVTransport",
                    av_transport_service_);
  }
  std::string time = SecondsFormatTime(play_duration_);
  av_transport_service_->SetStateVariable("RelativeTimePosition", time.c_str());
}

void SimulationMediaRenderer::StartTimer() {
  if (!play_timer_.IsRunning()) {
    // 开始定时查询任务的状态
    play_timer_.Start(FROM_HERE, base::Seconds(1),
                      base::BindRepeating(&SimulationMediaRenderer::PlayTimer,
                                          base::Unretained(this)));
  }
}

void SimulationMediaRenderer::StopTimer() {
  if (play_timer_.IsRunning()) {
    play_timer_.Stop();
  }
}

void SimulationMediaRenderer::PlayTimer() {
  play_duration_++;
}

std::string SimulationMediaRenderer::SecondsFormatTime(double seconds) {
  int hours = static_cast<int>(seconds) / 3600;
  int minutes = (static_cast<int>(seconds) % 3600) / 60;
  int remaining_seconds = static_cast<int>(seconds) % 60;

  char buffer[9];
  std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes,
                remaining_seconds);
  return std::string(buffer);
}

int SimulationMediaRenderer::TimeToSeconds(const std::string& formatted_time) {
  int hours, minutes, seconds;
  char colon;

  std::istringstream stream(formatted_time);
  stream >> hours >> colon >> minutes >> colon >> seconds;

  return (hours * 3600) + (minutes * 60) + seconds;
}
