// copyright 2022 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2022/09/13 14:30

#ifndef URL_FETCHER_WRAPPER_H_
#define URL_FETCHER_WRAPPER_H_

#include "base/task/single_thread_task_runner.h"

#include "base/logging.h"
#include "base/synchronization/waitable_event.h"

#include "base/run_loop.h"
#include "base/threading/thread.h"
#include "url/gurl.h"
#include "url/scheme_host_port.h"
#include "url/url_util.h"

#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_fetcher_delegate.h"
#include "net/url_request/url_request_context.h"

#include "nas/common/url_fetcher_wrapper/downloader_request_context_getter.h"


class FetcherWrapper : public net::URLFetcherDelegate {
 public:

   enum RequestType {
     GET,
     POST,
   };

  FetcherWrapper(base::WaitableEvent* complete_event);

  FetcherWrapper(const FetcherWrapper&) = delete;
  FetcherWrapper& operator=(const FetcherWrapper&) = delete;

  void SetHeader(const std::map<std::string, std::string>& header);
  void CreateFetcher(const GURL& url,
                     RequestType request_type);
  void SetUploadData(const std::string &upload_content_type, const std::string &upload_content);

  void SaveResponseToFileAtPath(const base::FilePath& path);
  bool GetResponseAsFilePath(base::FilePath* save_file);
  bool GetResponseString(std::string * response);
  int GetResponseCode();

  void Start();

  // Cancels the fetch by deleting the fetcher.
  void CancelFetch();

  // URLFetcherDelegate:
  void OnURLFetchComplete(const net::URLFetcher* source) override;
  void OnURLFetchDownloadProgress(const net::URLFetcher* source,
                                  int64_t current,
                                  int64_t total,
                                  int64_t current_network_bytes) override;
  void OnURLFetchUploadProgress(const net::URLFetcher* source,
                                int64_t current,
                                int64_t total) override;

  bool IsComplete() const { return did_complete_; }

  void SetCompleteClosure(base::OnceCallback<void(bool)> closure) {
    on_complete_or_cancel_ = std::move(closure);
  }
  void SetProgressClosure(base::RepeatingCallback<void(int percent)> closure) {
    on_download_progress_ = std::move(closure);
  }

 private:
  base::WaitableEvent* complete_event_ = nullptr;
  base::FilePath rsp_save_file_path_;
  bool did_complete_;
  std::unique_ptr<net::URLFetcher> fetcher_;
  base::OnceCallback<void(bool)> on_complete_or_cancel_;
  base::RepeatingCallback<void(int percent)> on_download_progress_;

  base::TimeTicks tick_;
};

#endif  // URL_FETCHER_WRAPPER_H_