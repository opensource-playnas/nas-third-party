/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-08-17 10:54:55
 */

#include "nas/common/url_fetcher_wrapper/simple_url_fetcher.h"

#include "nas/common/url_fetcher_wrapper/url_fetcher_wrapper.h"
#include "net/http/http_status_code.h"

namespace common {

SimpleUrlFetcher::SimpleUrlFetcher(
    scoped_refptr<base::SingleThreadTaskRunner> task_runner)
    : task_runner_(task_runner) {}
SimpleUrlFetcher::~SimpleUrlFetcher() {}

SimpleUrlFetcher::RequestId SimpleUrlFetcher::StartGet(
    const std::string& url,
    UrlFetcherRequestHeader request_header,
    UrlResponseCallback response_callback) {
  SimpleUrlFetcher::RequestId id = GenerateNewId();
  task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&SimpleUrlFetcher::CreateRequest, shared_from_this(), id,
                     url, net::URLFetcher::RequestType::GET, request_header,
                     std::move(response_callback), ""));
  return id;
}

void SimpleUrlFetcher::CreateRequest(
    const SimpleUrlFetcher::RequestId id,
    const std::string& url,
    net::URLFetcher::RequestType type,
    const UrlFetcherRequestHeader& request_header,
    UrlResponseCallback response_callback,
    const std::string& post_data) {
  DCHECK(task_runner_->RunsTasksInCurrentSequence());

  auto tag = net::DefineNetworkTrafficAnnotation("nas", "Nas url request");
  SimpleFetcherDelegate* delegate = new SimpleFetcherDelegate(this);
  UniqueURLFetcherPtr fetcher =
      net::URLFetcher::Create(GURL(url), type, delegate, tag);
  if (fetcher) {
    for (auto iter : request_header) {
      fetcher->AddExtraRequestHeader(iter.first, iter.second);
    }
    if (type == net::URLFetcher::RequestType::POST) {
      fetcher->SetUploadData("application/x-www-form-urlencoded", post_data);
    }

    fetcher->SetRequestContext(
        common::DownloaderRequestContextGetter::GetInstance()
            ->GetURLRequestContextGetter()
            .get());
    if (!fetchers_) {
      fetchers_ = std::make_unique<RequestMap>();
    }
    LOG(INFO) << "start request id :" << static_cast<int>(id);
    (*fetchers_)[id] =
        std::make_pair(std::move(fetcher), std::move(response_callback));
    (*fetchers_)[id].first->Start();
  }
}

SimpleUrlFetcher::RequestId SimpleUrlFetcher::StartPost(
    const std::string& url,
    const std::string& post_data,
    UrlFetcherRequestHeader request_header,
    UrlResponseCallback response_callback) {
  SimpleUrlFetcher::RequestId id = GenerateNewId();
  task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&SimpleUrlFetcher::CreateRequest, shared_from_this(), id,
                     url, net::URLFetcher::RequestType::POST, request_header,
                     std::move(response_callback), post_data));
  return id;
}

void SimpleUrlFetcher::OnRequestComplete(const net::URLFetcher* source) {
  if (!fetchers_)
    return;
  bool handler_complete = false;

  auto iter = std::find_if(
      fetchers_->begin(), fetchers_->end(),
                          [source](const auto& val) {
                            return val.second.first.get() == source;
                          });


  if (iter == fetchers_->end()) {
    return;
  }

  LOG(INFO) << "end request id :" << static_cast<int>(iter->first);
  int code = source->GetResponseCode();
  net::Error err = source->GetError();
  bool is_success = false;
  std::string response_body;
  if (code == net::HttpStatusCode::HTTP_OK) {
    source->GetResponseAsString(&response_body);
    is_success = true;
  }

  handler_complete = true;
  if (!iter->second.second.is_null()) {
    std::move(iter->second.second)
        .Run(is_success, static_cast<int>(err), response_body);
  }

  fetchers_->erase(iter);
  DCHECK(handler_complete);
  if (!handler_complete) {
    LOG(ERROR) << "no found request handler:" << source->GetOriginalURL();
  }
}
void SimpleUrlFetcher::SimpleFetcherDelegate::OnURLFetchComplete(
    const net::URLFetcher* source) {
  if (parent_) {
    parent_->OnRequestComplete(source);
  }

  // 网络请求返回后释放自己
  delete this;
}

void SimpleUrlFetcher::CancelAll() {
  if (task_runner_->RunsTasksInCurrentSequence()) {
    if (!fetchers_)
      return;
    fetchers_->clear();
  } else {
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&SimpleUrlFetcher::CancelAll, shared_from_this()));
  }
}

void SimpleUrlFetcher::CancelFetcher(RequestId request_id) {
  if (task_runner_->RunsTasksInCurrentSequence()) {
    if (!fetchers_)
      return;

    auto iter = fetchers_->find(request_id);
    if (iter != fetchers_->end()) {
      // 移除的时候会自动取消请求
      fetchers_->erase(iter);
    }
  } else {
    task_runner_->PostTask(FROM_HERE,
                           base::BindOnce(&SimpleUrlFetcher::CancelFetcher,
                                          shared_from_this(), request_id));
  }
}

SimpleUrlFetcher::RequestId SimpleUrlFetcher::GenerateNewId() {
  return next_id_++;
}
}  // namespace common