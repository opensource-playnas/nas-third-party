// copyright 2022 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2022/09/13 14:30

#include "url_fetcher_wrapper.h"
#include "base/threading/thread.h"
#include "base/threading/thread_task_runner_handle.h"
#include "nas/common/url_fetcher_wrapper/downloader_request_context_getter.h"
FetcherWrapper::FetcherWrapper(base::WaitableEvent* complete_event)
    : did_complete_(false), complete_event_(complete_event) {
  net::URLFetcher::SetIgnoreCertificateRequests(true);
}

void FetcherWrapper::SetHeader(
    const std::map<std::string, std::string>& header) {
  if (fetcher_) {
    for (auto iter : header) {
      fetcher_->AddExtraRequestHeader(iter.first, iter.second);
    }
  }
}

void FetcherWrapper::CreateFetcher(const GURL& url, RequestType request_type) {
  auto tag = net::DefineNetworkTrafficAnnotation("nas", "Nas url request");

  fetcher_.reset();
  fetcher_ = net::URLFetcher::Create(url,
                                     request_type == GET
                                         ? net::URLFetcher::RequestType::GET
                                         : net::URLFetcher::RequestType::POST,
                                     this, tag);
  if (!fetcher_) {
    if (complete_event_) {
      complete_event_->Signal();
    }
    return;
  }

  fetcher_->SetRequestContext(
      common::DownloaderRequestContextGetter::GetInstance()
          ->GetURLRequestContextGetter()
          .get());
}

void FetcherWrapper::SetUploadData(const std::string& upload_content_type,
                                   const std::string& upload_content) {
  if (fetcher_) {
    fetcher_->SetUploadData(upload_content_type, upload_content);
  }
}

int FetcherWrapper::GetResponseCode() {
  int response_code = 0;
  if (fetcher_) {
    response_code = fetcher_->GetResponseCode();
  }
  return response_code;
}

void FetcherWrapper::SaveResponseToFileAtPath(const base::FilePath& path) {
  if (fetcher_) {
    fetcher_->SaveResponseToFileAtPath(
        path, scoped_refptr<base::SequencedTaskRunner>(
                  base::SequencedTaskRunnerHandle::Get()));
  }
}

bool FetcherWrapper::GetResponseAsFilePath(base::FilePath* save_file) {
  if (fetcher_) {
    return fetcher_->GetResponseAsFilePath(true, save_file);
  }
  return false;
}

bool FetcherWrapper::GetResponseString(std::string* response) {
  if (fetcher_ && response) {
    return fetcher_->GetResponseAsString(response);
  }
  return false;
}

void FetcherWrapper::Start() {
  if (complete_event_) {
    complete_event_->Reset();
  }
  if (fetcher_) {
    LOG(INFO) << "will url fetcher...";
    fetcher_->Start();
  } else if (complete_event_) {
    complete_event_->Signal();
  }
}

void FetcherWrapper::CancelFetch() {
  fetcher_.reset();
  if (on_complete_or_cancel_) {
    std::move(on_complete_or_cancel_).Run(false);
  }
}

void FetcherWrapper::OnURLFetchComplete(const net::URLFetcher* source) {
  did_complete_ = true;
  if (complete_event_) {
    complete_event_->Signal();
  }
  auto fetch_erro = source->GetError();
  auto fetch_Code = source->GetResponseCode();
  LOG(INFO) << "fetch_erro: " << fetch_erro
            << ", err_msg: " << net::ErrorToString(fetch_erro)
            << ", fetch_Code: " << fetch_Code;

  if (on_complete_or_cancel_) {
    std::move(on_complete_or_cancel_)
        .Run(fetch_erro == net::OK && fetch_Code == 200);
  }
  LOG(INFO) << "OnURLFetchComplete";
}

void FetcherWrapper::OnURLFetchDownloadProgress(const net::URLFetcher* source,
                                                int64_t current,
                                                int64_t total,
                                                int64_t current_network_bytes) {
  // Note that the current progress may be greater than the previous progress,

  // If file size is not known, |total| is -1.
  /*LOG(INFO) << "OnURLFetchDownloadProgress, total: " << total
            << ", current: " << current;*/
  if (!on_download_progress_) {
    return;
  }

  int percent = 0;
  if (total > 0) {
    percent = current * 100 / total;
  }
  auto now_time_tick = base::TimeTicks::Now();
  if (percent == 100 || now_time_tick - tick_ < base::Seconds(1)) {
    return;
  }
  tick_ = base::TimeTicks::Now();
  on_download_progress_.Run(percent);
}

void FetcherWrapper::OnURLFetchUploadProgress(const net::URLFetcher* source,
                                              int64_t current,
                                              int64_t total) {
  // Note that the current progress may be greater than the previous progress,
  // in the case of retrying the request.

  LOG(INFO) << "OnURLFetchUploadProgress, total: " << total
            << ", current: " << current;
}
