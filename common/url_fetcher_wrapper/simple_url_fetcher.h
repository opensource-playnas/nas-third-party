/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-08-17 10:12:09
 */

#ifndef NAS_COMMON_URL_FETCHER_WRAPPER_SIMPLE_URL_FETCHER_H_
#define NAS_COMMON_URL_FETCHER_WRAPPER_SIMPLE_URL_FETCHER_H_

#include <map>

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/threading/thread_restrictions.h"
#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_fetcher_delegate.h"
#include "net/url_request/url_request_context_getter.h"

namespace common {
using UrlFetcherRequestHeader = std::map<std::string, std::string>;
using UrlResponseCallback = base::OnceCallback<
    void(bool is_success, int net_error, const std::string& response)>;

using UniqueURLFetcherPtr = std::unique_ptr<net::URLFetcher>;
class SimpleUrlFetcher: public std::enable_shared_from_this<SimpleUrlFetcher> {
 public:
  SimpleUrlFetcher(scoped_refptr<base::SingleThreadTaskRunner> task_runner);
  ~SimpleUrlFetcher();

  using RequestId = int64_t;
  RequestId StartGet(const std::string& url,
                     UrlFetcherRequestHeader request_header,
                     UrlResponseCallback response_callback);

  RequestId StartPost(const std::string& url,
                      const std::string& post_data,
                      UrlFetcherRequestHeader request_header,
                      UrlResponseCallback response_callback);

  void CancelFetcher(RequestId request_id);
  void CancelAll();
 private:
   class SimpleFetcherDelegate : public net::URLFetcherDelegate {
      public:
        SimpleFetcherDelegate(SimpleUrlFetcher* parent): parent_(parent){}; 
        void OnURLFetchComplete(const net::URLFetcher* source) override;
    
      private:
        SimpleUrlFetcher* parent_;

   }; // SimpleFetcherDelegate

  void OnRequestComplete(const net::URLFetcher* source);

  void CreateRequest(
      const SimpleUrlFetcher::RequestId id,
      const std::string& url,
      net::URLFetcher::RequestType type,
      const UrlFetcherRequestHeader& request_header,
      UrlResponseCallback response_callback,
      const std::string& post_data);
  RequestId GenerateNewId();

 private:
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;
  // 保存UniqueURLFetcherPtr,确保持有指针,避免对象被释放
  using RequestMap = std::map<RequestId,
           std::pair<std::unique_ptr<net::URLFetcher>, UrlResponseCallback>>;
  using UniqueURLFetcherMapPtr = std::unique_ptr<RequestMap>;
  UniqueURLFetcherMapPtr fetchers_;

  // 每个对象自己维护自己的id
  std::atomic<RequestId> next_id_ = 0;
};  // SimpleUrlFetcher
}  // namespace common
#endif  // NAS_COMMON_URL_FETCHER_WRAPPER_SIMPLE_URL_FETCHER_H_