/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-08-17 21:14:12
 */

#ifndef NAS_COMMON_URL_FETCHER_WRAPPER_DOWNLOADER_REQUEST_CONTEXT_GETTER_H_
#define NAS_COMMON_URL_FETCHER_WRAPPER_DOWNLOADER_REQUEST_CONTEXT_GETTER_H_

#include "base/task/single_thread_task_runner.h"

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/threading/thread.h"
#include "base/threading/thread_task_runner_handle.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_builder.h"
#include "net/url_request/url_request_context_getter.h"
namespace common {

class DownloaderRequestContextGetter {
 public:
  static DownloaderRequestContextGetter* GetInstance() {
    return base::Singleton<DownloaderRequestContextGetter>::get();
  }
  void Init();
  void Uninit();
  static std::unique_ptr<net::ProxyConfigService> GetNasProxyConfig();
  scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner();
  class NasURLRequestContextGetter : public net::URLRequestContextGetter {
   public:
    net::URLRequestContext* GetURLRequestContext() override;

    scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner()
        const override;

   protected:
    ~NasURLRequestContextGetter() override {
      // |context_| may only be deleted on the network thread. Fortunately,
      // the parent class already ensures it's deleted on the network thread.
      DCHECK(network_task_runner_->RunsTasksInCurrentSequence());
      url_request_context_.reset();
      NotifyContextShuttingDown();
      if (!on_destruction_callback_.is_null())
        std::move(on_destruction_callback_).Run();
    }

    // Sets callback to be invoked when the getter is destroyed.
    void set_on_destruction_callback(
        base::OnceClosure on_destruction_callback) {
      on_destruction_callback_ = std::move(on_destruction_callback);
    }

   private:
    friend class DownloaderRequestContextGetter;
    void SetData(scoped_refptr<base::SingleThreadTaskRunner>,
                 std::unique_ptr<net::URLRequestContext>);
   private:
    std::unique_ptr<net::URLRequestContext> url_request_context_;
    scoped_refptr<base::SingleThreadTaskRunner> network_task_runner_;
    base::OnceClosure on_destruction_callback_;
  };  // NasURLRequestContextGetter
  
  scoped_refptr<NasURLRequestContextGetter> GetURLRequestContextGetter();
 private:
  friend struct base::DefaultSingletonTraits<DownloaderRequestContextGetter>;

  DownloaderRequestContextGetter();
  DownloaderRequestContextGetter(const DownloaderRequestContextGetter&) =
      delete;
  DownloaderRequestContextGetter& operator=(
      const DownloaderRequestContextGetter&) = delete;

  // Tells the getter to act as if the URLRequestContext is about to be shut
  // down.
  void Shutdown();

 private:
  std::shared_ptr<base::Thread> network_thread_;
  scoped_refptr<NasURLRequestContextGetter> url_reqeust_context_getter_;

  bool shutting_down_ = false;
};
}  // namespace common
#endif  // NAS_COMMON_URL_FETCHER_WRAPPER_DOWNLOADER_REQUEST_CONTEXT_GETTER_H_