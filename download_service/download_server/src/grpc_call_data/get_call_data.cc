#include "get_call_data.h"
#include "nas/download_service/download_server/src/download_mgr.h"

namespace nas {

GetCallData::GetCallData(nas::DownloadServiceImpl* service,
                         ServerCompletionQueue* cq,
                         NasThread* cq_dispatch,
                         NasThread* task_main)
    : nas::GrpcServerCallBase<nas::DownloadServiceImpl, GetCallData>(
          service,
          cq,
          cq_dispatch,
          task_main),
      responder_(&ctx_) {}

void GetCallData::Request(DownloadServiceImpl* service,
                          ServerCompletionQueue* cq) {
  if (service && cq && tag_) {
    service->RequestGet(&ctx_, &request_, &responder_, cq, cq, tag_);
  }
}

void GetCallData::Process() {
  nas::ContextField context_field(&ctx_);
  std::vector<TaskInfoPtr> task_info_list;

  if (service_) {
    std::shared_ptr<nas::DownloadManager> download_mgr =
        service_->GetDownloadMgr();
    if (download_mgr) {
        download_mgr->GetTaskInfoList(context_field, base::BindOnce(
              &GetCallData::ProcessComplete, base::Unretained(this)));
    }
  }
}

void GetCallData::ProcessComplete(const std::vector<TaskInfoPtr>& task_info_list) {
   for (TaskInfoPtr task_info : task_info_list) {
    download_data_struct::DownloadTaskInfo* info =
        response_.add_task_info_list();
    TaskParamPtr task_param = task_info->GetTaskParam();
    TaskBaseInfoPtr task_base_info = task_info->GetTaskBaseInfo();
    TaskProgressInfoPtr progress_info = task_info->GetProgressInfo();
    TaskTimeStampPtr task_time_stamp = task_info->GetTaskTimeStamp();

    info->set_task_id(task_param->id);
    info->set_source(task_param->url);
    info->set_save_path(task_param->save_path);
    info->set_task_status(task_info->GetTaskStatus());
    info->set_task_category(task_info->GetTaskCategory());
    info->set_task_is_valid(task_info->IsValid());
    info->set_file_list_info(task_base_info->GetFileList());

    FileAttribute file_attribute = task_base_info->GetTaskAttribute();
    info->set_name(file_attribute.file_name);
    info->set_total_size(std::to_string(file_attribute.file_size));
    info->set_total_progress(progress_info->GetTotalProgress());
    info->set_download_speed(progress_info->GetDownloadSpeed());
    info->set_upload_speed(progress_info->GetUploadSpeed());
    double completed_size =
        file_attribute.file_size * progress_info->GetTotalProgress() / 100.0;
    info->set_completed_size(std::to_string(completed_size));

    double time_remaining = -1;
    if (progress_info->GetDownloadSpeed() != 0) {
      time_remaining = (file_attribute.file_size - completed_size) /
                       progress_info->GetDownloadSpeed();  // 秒
    }

    info->set_time_remaining(time_remaining);
    info->set_create_timestamp(std::to_string(task_time_stamp->create_time));
    info->set_finish_timestamp(std::to_string(task_time_stamp->finish_time));
    info->set_delete_timestamp(std::to_string(task_time_stamp->delete_time));

    // if (task_param->converted_url.empty()) {
    //   task_param->converted_url = task_param->url;
    // }

    info->set_converted_source(task_param->converted_url);
    info->set_file_type(file_attribute.file_type);
    info->set_download_type(task_param->type);
    info->set_local_land_name(file_attribute.local_land_file_name);
    info->set_err_code(progress_info->GetErrorCode());
    nas::CallResult call_result =
        utils::ErrNoToCallResult(progress_info->GetErrorCode());
    info->set_msg(call_result.msg);
  }

  nas::WrapperSetResult wr(&ctx_);
  wr.SetResult(call_request_);
  OnProcessed();
}

void GetCallData::Finish() {
  if (tag_) {
    responder_.Finish(response_, grpc::Status::OK, tag_);
  }
}

}  // namespace nas