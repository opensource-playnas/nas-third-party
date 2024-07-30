/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: fengbangyao@ludashi.com
* @Date: 2023-01-16 10:23:36
*/

#include "testing/gtest/include/gtest/gtest.h"

#include "nas/common/url_fetcher_wrapper/url_fetcher_helper.h"
#include "nas/common/encrypt/encrypt_blowfish_base64.h"

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/escape.h"
#include "base/logging.h"
#include "base/task/thread_pool.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/threading/thread.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/time/clock.h"

static const char* kEncryptKey = "cb49dc28-f006-4ecf-9907-d756e4647b4a";
static const char* kVerifyTokenApi = "http://playnas.com/api/pc/token/verify";
namespace nas{
// 测试网络库

class ThreadPool{
  public:
  ThreadPool(){    
    base::ThreadPoolInstance::Create("Url_Fetcher_Helper_Test");
    base::ThreadPoolInstance::Get()->Start({50});
  }

  ~ ThreadPool(){
    base::ThreadPoolInstance::Get()->Shutdown();
  }
};
ThreadPool pool;

TEST(Url_Fetcher_Helper_Test, DownloadString){
 base::Value::List token_list;
 std::vector<std::string> access_tokens;

 access_tokens.push_back("eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJodHRwOlwvXC9wbGF5bmFzLmNvbVwvYXBpXC90b2tlblwvYWNjZXNzIiwiaWF0IjoxNjczOTE5Njk4LCJleHAiOjE2NzY1MTE2OTgsIm5iZiI6MTY3MzkxOTY5OCwianRpIjoiU2paUVMyQ1o5VFJiYUFYSCIsInN1YiI6NywicHJ2IjoiYTRjNDg4YTkwNzBkMzA1MWVjODJlYWJjOWJhNmNkZjIxZWQ2NTUzYyJ9.iLcskeqsGXETtS_4Sc6CIZkYg8ywprBhOLb85Dy6GAk");
  for (auto& iter : access_tokens) {
    if (!iter.empty()) {
      token_list.Append(iter);
    }
  }

  base::Value::Dict token_dict;
  token_dict.Set("access_tokens", std::move(token_list));

  std::string request_json;
  base::JSONWriter::Write(token_dict, &request_json);

  LOG(INFO) << "BatchVerifyToken request_json:" << request_json;
  std::string clipher_text;
  Encrypt_Blowfish_Base64(request_json, kEncryptKey, clipher_text);

  std::string request_data("data=");
  request_data.append(base::EscapeUrlEncodedData(clipher_text, false));

  std::string verify_api = kVerifyTokenApi;

  int test_count = 50;
  do{   
    std::string verify_response;
    base::Time start = base::Time::Now();
    int response_code = 0;
    bool request_result = UrlFetcherHelper::SyncPostString(
        verify_api, request_data, {}, base::Seconds(20), &verify_response,&response_code);
      
    base::Time end = base::Time::Now();
    base::TimeDelta delta = end - start;
    std::cout<<"test_count:"<<test_count<<",request interval:"<<delta.InMilliseconds()<<std::endl;

    if (request_result) {
      std::string response_data;
      Decrypt_Base64_Blowfish( 
          verify_response, kEncryptKey, response_data);
      LOG(INFO)<<"verify access_token response:"<<response_data;
      HttpResponseData res_data(response_data);
    }

  }while(test_count--);

}
} // namespace nas