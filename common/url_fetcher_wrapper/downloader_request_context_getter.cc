/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-08-17 21:17:35
 */
#include "nas/common/url_fetcher_wrapper/downloader_request_context_getter.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/synchronization/waitable_event.h"

#include "net/proxy_resolution/proxy_config_service_fixed.h"

#include "nas/common/nas_config.h"

#if BUILDFLAG(IS_WIN)
#include "net/proxy_resolution/win/proxy_config_service_win.h"
#endif

namespace common {
DownloaderRequestContextGetter::DownloaderRequestContextGetter() {
  network_thread_ = std::make_unique<base::Thread>("download_network_thread");
  base::Thread::Options network_thread_options;
  network_thread_options.message_pump_type = base::MessagePumpType::IO;
  bool result =
      network_thread_->StartWithOptions(std::move(network_thread_options));
  DCHECK(result);
}

void DownloaderRequestContextGetter::Init() {
  base::WaitableEvent waiter;
  url_reqeust_context_getter_ =
      base::MakeRefCounted<NasURLRequestContextGetter>();
  network_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](DownloaderRequestContextGetter* getter,
             base::WaitableEvent* waiter) {
            auto builder = std::make_unique<net::URLRequestContextBuilder>();
            builder->set_proxy_config_service(
                DownloaderRequestContextGetter::GetNasProxyConfig());
            builder->set_throttling_enabled(true);
            getter->url_reqeust_context_getter_->SetData(
                base::ThreadTaskRunnerHandle::Get(), builder->Build());
            waiter->Signal();
          },
          base::Unretained(this), &waiter));
  waiter.Wait();
}

scoped_refptr<base::SingleThreadTaskRunner>
DownloaderRequestContextGetter::GetNetworkTaskRunner() {
  return network_thread_->task_runner();
}

scoped_refptr<DownloaderRequestContextGetter::NasURLRequestContextGetter>
DownloaderRequestContextGetter::GetURLRequestContextGetter() {
  return url_reqeust_context_getter_;
}

void DownloaderRequestContextGetter::NasURLRequestContextGetter::SetData(
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    std::unique_ptr<net::URLRequestContext> context) {
  network_task_runner_ = task_runner;
  url_request_context_ = std::move(context);
}

scoped_refptr<base::SingleThreadTaskRunner> DownloaderRequestContextGetter::
    NasURLRequestContextGetter::GetNetworkTaskRunner() const {
  return network_task_runner_;
}

net::URLRequestContext* DownloaderRequestContextGetter::
    NasURLRequestContextGetter::GetURLRequestContext() {
  return url_request_context_.get();
}

void DownloaderRequestContextGetter::Uninit() {
  Shutdown();
}

std::unique_ptr<net::ProxyConfigService>
DownloaderRequestContextGetter::GetNasProxyConfig() {
  do {
    // 为了方便使用fiddler
    // 抓包，可以在public目录下设置成fiddler的代理文件
    base::FilePath sys_public = nas::GetNasConfigDir();
    sys_public = sys_public.DirName();
    base::FilePath nas_proxy =
        sys_public.Append(FILE_PATH_LITERAL("nas_proxy"));
    if (!base::PathExists(nas_proxy)) {
      LOG(WARNING) << "non nas_proxy file";
      break;
    }
    std::string content;
    if (!base::ReadFileToString(nas_proxy, &content) || content.empty()) {
      LOG(WARNING) << "nas_proxy content is empty";
      break;
    }
    LOG(WARNING) << "nas_proxy: " << content;
    net::ProxyConfig proxy_cfg;
    // http=127.0.0.1:8888;https=127.0.0.1:8888
    proxy_cfg.proxy_rules().ParseFromString(content);

    return std::make_unique<net::ProxyConfigServiceFixed>(
        net::ProxyConfigWithAnnotation(proxy_cfg,
                                       net::DefineNetworkTrafficAnnotation(
                                           "NAS_DNTA", R"(nas_dnta: todo)")));

#if BUILDFLAG(IS_WIN)
    /*builder->set_proxy_config_service(
               std::make_unique<net::ProxyConfigServiceWin>(
                   net::DefineNetworkTrafficAnnotation("NAS_DNTA",
                                                       R"(nas_dnta: todo)")));*/
#endif
  } while (false);

  return std::make_unique<net::ProxyConfigServiceFixed>(
      net::ProxyConfigWithAnnotation::CreateDirect());
}

void DownloaderRequestContextGetter::Shutdown() {
  //   if (!network_thread_->task_runner()->RunsTasksInCurrentSequence()) {
  //     network_thread_->task_runner()->PostTask(
  //         FROM_HERE,
  //         base::BindOnce(&DownloaderRequestContextGetter::Shutdown, this));
  //     return;
  //   }

  shutting_down_ = true;
  url_reqeust_context_getter_.reset();
}

}  // namespace common
