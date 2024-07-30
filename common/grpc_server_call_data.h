// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author heyiqian@ludashi.com
// date 2023-07-13 14:43:33

#ifndef NAS_COMMON_GRPC_SERVER_CALL_DATA_H_
#define NAS_COMMON_GRPC_SERVER_CALL_DATA_H_

#include <grpcpp/impl/codegen/completion_queue.h>

#include "base/task/thread_pool.h"
#include "base/task/thread_pool/thread_pool_instance.h"

#include "nas/common/nas_thread.h"

// clang-format off
// 具体的grpc接口实现继承该抽象类
// 1 外部需传入2个线程
//   线程1: kGrpcCQDispatchThreadName, 任务调度，Proceed 都在该线程执行，grpc接口实现的结果再回调到该线程
//   线程2: kGrpcTaskMainThreadName, 处理grpc实际的task, 各个模块中是否再将任务分发由各模块自行决定
// 2 grpc接口要被执行，需先请求grpc接口（注册请求）
// 3 HandleRpcs 在初试完grpc服务、注册完请求之后调用

// demo:
/*
RunGrpcServer() {
  string server_address("127.0.0.1:0");
  grpc::EnableDefaultHealthCheckService(true);
  // grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  grpc::ServerBuilder builder;
  int listen_port = 0;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials(),
                           &listen_port);
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  // TService，grpc服务的AsyncService版本。
  // 我们的代码中一般用this，则需继承自AsyncService版本。
  TService service; 
  builder.RegisterService(&service);
  // register grpc health check
  nas::GrpcHealthRegister(ServiceName, builder);

  // 我们的代码业务需要
  // nas::GrpcBaseServiceImpl grpc_base_service_impl(this);
  // builder.RegisterService(&grpc_base_service_impl);

  // 添加完成队列
  // 我们的实际使用中cq定义为成员变量，shutdown的时候需通过它调用
  cq = builder.AddCompletionQueue();
  // Finally assemble the server.
  // 我们的实际使用中server定义为成员变量，shutdown的时候需通过它调用
  std::unique_ptr<grpc::Server> server = builder.BuildAndStart();

  // 请求注册，每个接口都需要请求
  CreateCallDatas();
  HandleRpcs(cq->get());
}
*/
// clang-format on

namespace nas {

// grpc 服务端用到的两个线程名
static const char kGrpcCQDispatchThreadName[] = "grpc_cq_dispatch";
static const char kGrpcTaskMainThreadName[] = "grpc_task_main";

class GrpcServerCallData {
 public:
  virtual ~GrpcServerCallData() {}
  virtual void Proceed() = 0;
};

template <typename TService, typename TCallData>
void NewCallData(TService* service,
                 grpc::ServerCompletionQueue* cq,
                 NasThread* cq_dispatch,
                 NasThread* task_main) {
  auto call_data = new TCallData(service, cq, cq_dispatch, task_main);
  // 第一次调用 Proceed 走 Create 流程
  call_data->Proceed();
}

template <typename TService, typename TCallData>
class GrpcServerCallDataBase : public GrpcServerCallData {
 public:
  enum GrpcCallDataStatus { kCreate, kProcess, kFinish };
  GrpcServerCallDataBase(TService* service,
                         grpc::ServerCompletionQueue* cq,
                         NasThread* cq_dispatch,
                         NasThread* task_main)
      : service_(service),
        cq_(cq),
        status_(kCreate),
        cq_dispatch_(cq_dispatch),
        task_main_(task_main) {
    DCHECK(cq_dispatch_);
    DCHECK(task_main_);
  }

  void Proceed() {
    if (!cq_dispatch_->IsCurrentThread()) {
      cq_dispatch_->PostTask(base::BindOnce(&GrpcServerCallDataBase::Proceed,
                                            base::Unretained(this)));
      return;
    }

    switch (status_) {
      case kCreate:
        status_ = kProcess;
        Request(service_, cq_);
        break;
      case kProcess:
        // new 一个新的 obj, 开始下一次请求
        NewCallData<TService, TCallData>(service_, cq_, cq_dispatch_,
                                         task_main_);
        task_main_->PostTask(base::BindOnce(&GrpcServerCallDataBase::Process,
                                            base::Unretained(this)));
        break;
      case kFinish:
        delete this;
        break;
      default:
        break;
    }
  }

 protected:
  // 需派生类实现 ------
  // 注册请求
  virtual void Request(TService* service, grpc::ServerCompletionQueue* cq) = 0;
  // 处理操作，对请求的具体执行, 在线程池中执行, 处理完成后调用 OnProcessed
  virtual void Process() = 0;
  // 完成工作
  virtual void Finish() = 0;
  // 需派生类实现 ------ end

  void OnProcessed() {
    if (!cq_dispatch_->IsCurrentThread()) {
      cq_dispatch_->PostTask(base::BindOnce(
          &GrpcServerCallDataBase::OnProcessed, base::Unretained(this)));
      return;
    }

    status_ = kFinish;
    Finish();
  }

 protected:
  // 子类会从TService获取全局性的对象或方法
  TService* service_;

 private:
  grpc::ServerCompletionQueue* cq_;
  GrpcCallDataStatus status_;
  // 所有 call data 的 proceed 都在该线程中执行
  NasThread* cq_dispatch_ = nullptr;
  // 每个请求的处理操作(Process), 该线程作为主入口
  // grpc server是否要分发到其他线程由自身决定
  NasThread* task_main_ = nullptr;
};

// 替换完成把之前的都删掉
using ProceedCallBack = base::RepeatingCallback<void()>;

class TagHandler {
 public:
  TagHandler(ProceedCallBack call_back) { call_back_ = std::move(call_back); }

  void Proceed() { call_back_.Run(); }

 private:
  ProceedCallBack call_back_;
};

// 统一使用下面的GrpcServerCallBase和NewCallData，因为NewCallData上面有定义，所以
// 屏蔽了，用完把上面的GrpcServerCallDataWrapper和GrpcServerCallDataBase相关都删掉,
// 然后把下面NewCallData放开
// template <typename TService, typename TCallData>
// void NewCallData(TService* service,
//                  grpc::ServerCompletionQueue* cq,
//                  NasThread* cq_dispatch,
//                  NasThread* task_main) {
//   auto call_data = new TCallData(service, cq, cq_dispatch, task_main);
//   // 第一次调用 Proceed 走 Create 流程
//   call_data->Proceed();
// }
template <typename TService, typename TCallData>
class GrpcServerCallBase : public GrpcServerCallData {
 public:
  enum GrpcCallDataStatus { kCreate, kProcess, kFinish };
  GrpcServerCallBase(TService* service,
                         grpc::ServerCompletionQueue* cq,
                         NasThread* cq_dispatch,
                         NasThread* task_main)
      : service_(service),
        cq_(cq),
        status_(kCreate),
        cq_dispatch_(cq_dispatch),
        task_main_(task_main) {
    DCHECK(cq_dispatch_);
    DCHECK(task_main_);
    tag_ = new TagHandler(base::BindRepeating(&GrpcServerCallBase::Proceed,
                                              base::Unretained(this)));
  }

  virtual ~GrpcServerCallBase() {
    if (tag_) {
      delete tag_;
    }
  }

  void Proceed() {
    if (!cq_dispatch_->IsCurrentThread()) {
      cq_dispatch_->PostTask(base::BindOnce(&GrpcServerCallBase::Proceed,
                                            base::Unretained(this)));
      return;
    }

    switch (status_) {
      case kCreate:
        status_ = kProcess;
        Request(service_, cq_);
        break;
      case kProcess:
        // new 一个新的 obj, 开始下一次请求
        NewCallData<TService, TCallData>(service_, cq_, cq_dispatch_,
                                         task_main_);
        task_main_->PostTask(base::BindOnce(&GrpcServerCallBase::Process,
                                            base::Unretained(this)));
        break;
      case kFinish:
        delete this;
        break;
      default:
        break;
    }
  }

 protected:
  // 需派生类实现 ------
  // 注册请求
  virtual void Request(TService* service,
                       grpc::ServerCompletionQueue* cq) = 0;
  // 处理操作，对请求的具体执行, 在线程池中执行, 处理完成后调用 OnProcessed
  virtual void Process() = 0;
  // 完成工作
  virtual void Finish() = 0;
  // 需派生类实现 ------ end

  void OnProcessed() {
    if (!cq_dispatch_->IsCurrentThread()) {
      cq_dispatch_->PostTask(base::BindOnce(
          &GrpcServerCallBase::OnProcessed, base::Unretained(this)));
      return;
    }

    status_ = kFinish;
    Finish();
  }

 protected:
  // 子类会从TService获取全局性的对象或方法
  TService* service_;
  TagHandler* tag_ = nullptr;

 private:
  grpc::ServerCompletionQueue* cq_;
  GrpcCallDataStatus status_;
  // 所有 call data 的 proceed 都在该线程中执行
  NasThread* cq_dispatch_ = nullptr;
  // 每个请求的处理操作(Process), 该线程作为主入口
  // grpc server是否要分发到其他线程由自身决定
  NasThread* task_main_ = nullptr;

};

// 进入队列循环
void HandleRpcs(grpc::ServerCompletionQueue* cq);

// 替换完成把之前的删掉 
void HandleRpcRequests(grpc::ServerCompletionQueue* cq);

}  // namespace nas

#endif