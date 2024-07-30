/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2022-11-21 10:47:10
 */
#include "nas/common/url_fetcher_wrapper/url_fetcher_helper.h"

#include <map>
#include <string>

#include "base/check.h"
#include "base/logging.h"
#include "base/threading/scoped_blocking_call.h"
#include "base/threading/scoped_thread_priority.h"
#include "base/threading/thread.h"

#include "nas/common/url_fetcher_wrapper/url_fetcher_wrapper.h"

namespace nas {
HttpResponseData::HttpResponseData(const std::string& response) {
  json_obj_ = base::JSONReader::Read(response);
}

bool HttpResponseData::IsValid() {
  return json_obj_ && json_obj_.has_value();
}

bool HttpResponseData::GetData(base::Value** value) {
  DCHECK(value);
  bool find = false;
  if (json_obj_ && json_obj_.has_value()) {
    base::Value::Dict* root_dict = json_obj_->GetIfDict();
    if (root_dict) {
      *value = root_dict->Find("data");
      if (*value) {
        find = true;
      }
    }
  }

  return find;
}

bool HttpResponseData::GetErrorNo(int* error_no) {
  DCHECK(error_no);
  bool find = false;

  if (json_obj_ && json_obj_.has_value()) {
    base::Value::Dict* root_dict = json_obj_->GetIfDict();
    if (root_dict) {
      absl::optional<int> int_value = root_dict->FindInt("errno");
      if (int_value && int_value.has_value()) {
        *error_no = int_value.value();
        find = true;
      }
    }
  }

  return find;
}

bool HttpResponseData::GetErrorMessage(std::string* msg) {
  DCHECK(msg);
  bool find = false;

  if (json_obj_ && json_obj_.has_value()) {
    base::Value::Dict* root_dict = json_obj_->GetIfDict();
    if (root_dict) {
      std::string* msg_value = root_dict->FindString("msg");
      if (msg_value) {
        *msg = *msg_value;
        find = true;
      }
    }
  }

  return find;
}

bool UrlFetcherHelper::SyncGetString(const std::string& url,
                                     std::map<std::string, std::string> header,
                                     base::TimeDelta delay,
                                     std::string* response) {
  bool request = false;

  base::WaitableEvent waiter;
  base::Thread sync_thread("sync_getstring_thread");
  sync_thread.StartWithOptions(
      base::Thread::Options{base::MessagePumpType::IO, 0});
  std::shared_ptr<FetcherWrapper> url_fetcher =
      std::make_shared<FetcherWrapper>(&waiter);
  sync_thread.task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](std::shared_ptr<FetcherWrapper> url_fetcher, std::string url,
             std::map<std::string, std::string> header) {
            url_fetcher->CreateFetcher(GURL(url),
                                       FetcherWrapper::RequestType::GET);
            url_fetcher->SetHeader(header);
            url_fetcher->Start();
          },
          url_fetcher, url, header));

  do {
    if (!waiter.TimedWait(delay)) {
      LOG(INFO) << "SyncGetString failed:" << url;
      sync_thread.task_runner()->PostTask(
          FROM_HERE, base::BindOnce(
                         [](std::shared_ptr<FetcherWrapper> fetcher) {
                           fetcher->CancelFetch();
                         },
                         url_fetcher));
      break;
    }

    int http_code = url_fetcher->GetResponseCode();
    if (http_code != 200) {
      LOG(ERROR) << " url fetcher error:" << http_code;
      break;
    }

    request = url_fetcher->GetResponseString(response);
    if (!request) {
      LOG(WARNING) << "get response string failed";
      break;
    }

  } while (false);
  sync_thread.task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](std::shared_ptr<FetcherWrapper> fetcher) { fetcher.reset(); },
          url_fetcher));

  return request;
}

bool UrlFetcherHelper::SyncPostString(const std::string& url,
                                      const std::string& data,
                                      std::map<std::string, std::string> header,
                                      base::TimeDelta delay,
                                      std::string* response,
                                      int* response_code) {
  bool request = false;

  base::WaitableEvent waiter;
  base::Thread sync_thread("sync_poststring_thread");
  sync_thread.StartWithOptions(
      base::Thread::Options{base::MessagePumpType::IO, 0});
  std::shared_ptr<FetcherWrapper> url_fetcher =
      std::make_shared<FetcherWrapper>(&waiter);
  sync_thread.task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](std::shared_ptr<FetcherWrapper> url_fetcher, std::string url,
             std::map<std::string, std::string> header, std::string data) {
            url_fetcher->CreateFetcher(GURL(url),
                                       FetcherWrapper::RequestType::POST);
            url_fetcher->SetHeader(header);
            url_fetcher->SetUploadData(
                "application/x-www-form-urlencoded",
                data);  // application/x-www-form-urlencoded
            url_fetcher->Start();
          },
          url_fetcher, url, header, data));

  do 
  {
    if (!waiter.TimedWait(delay)) {
      LOG(INFO) << "SyncPostString timeout:" << url;
      sync_thread.task_runner()->PostTask(
          FROM_HERE, base::BindOnce(
                         [](std::shared_ptr<FetcherWrapper> fetcher) {
                           fetcher->CancelFetch();
                         },
                         url_fetcher));
      break;
    }
    int http_code = url_fetcher->GetResponseCode();
    if (response_code) {
      *response_code = http_code;
    }
    if (http_code != 200) {
      LOG(ERROR) << " url fetcher error:" << http_code;
      break;
    }

    request = url_fetcher->GetResponseString(response);
    if (!request) {
      LOG(WARNING) << "get response string failed";
      break;
    }


  } while (false);

  

  sync_thread.task_runner()->PostTask(
      FROM_HERE, base::BindOnce(
                     [](std::shared_ptr<FetcherWrapper>& fetcher) {
                       if (fetcher) {
                         fetcher.reset();
                       }
                     },
                     std::ref(url_fetcher)));
  sync_thread.Stop();
  return request;
}

}  // namespace nas