// testAmuled.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>
#include <Shlwapi.h>
#include <future>         // std::async, std::future
#include <thread>
#include "IDownload.h"

#pragma comment(lib, "shlwapi.lib")

typedef IDownload*(__cdecl* PFNGetDownloadMgr)();
typedef int(__cdecl* PFNInit)(int, char* []);
typedef int(__cdecl* PFNUnInit)();

HMODULE InitNet(size_t nCurrentModuleDepth) {
  WCHAR szNet[MAX_PATH] = {};
  GetModuleFileName(NULL, szNet, MAX_PATH);
  PathRemoveFileSpec(szNet);
  for (size_t ii = 0; ii < nCurrentModuleDepth; ++ii) PathRemoveFileSpec(szNet);
#ifdef _WIN64
  PathAppend(szNet, L"aMuleD.dll");
#else
  PathAppend(szNet, L"aMuleD.dll");
#endif

  if (!PathFileExists(szNet)) {
    return NULL;
  }

  HMODULE handle = LoadLibrary(szNet);
  return handle;
}

IDownload* GetDownloadMgr() {
  HMODULE handle = InitNet(0);
  if (handle != NULL) {
    PFNGetDownloadMgr mgr =
        (PFNGetDownloadMgr)GetProcAddress(handle, "GetDownloadMgr");
    if (!mgr) {
      std::cout << "get download mgr func failed!" << std::endl;
      return nullptr;
    }
    return mgr();
  }
  return nullptr;
} 

int Init(int argc, char* argv[]) {
  HMODULE handle = InitNet(0);
  if (handle != NULL) {
    PFNInit p_init = (PFNInit)GetProcAddress(handle, "Init");
    if (!p_init) {
      std::cout << "get p_init func failed!" << std::endl;
      return 0;
    }
    return p_init(argc, argv);
  }
  return 0;
}

int UnInit() {
  HMODULE handle = InitNet(0);
  if (handle != NULL) {
    PFNUnInit p_uninit = (PFNUnInit)GetProcAddress(handle, "UnInit");
    if (!p_uninit) {
      std::cout << "get p_uninit func failed!" << std::endl;
      return 0;
    }
    return p_uninit();
  }
  return 0;
}

int Download() {
  Sleep(8000);
  IDownload* download_mgr = GetDownloadMgr();
  std::cout << "add download" << std::endl;
  DownloadParam param;
  param.id = 1;
  //std::string source = "ed2k://|file|(500)Pretty%20Nice%20Girl%20Pic%20美女图片(500张)让你看个够%20珍藏%20明星%20赵薇%20林心如%20写真%20裸体%20美女%20图片%20性%20色情%209P%20Yjl.zip|67252977|F64D4BFDBF2C72CBC67AAF525F0F0376|/";
  std::string source = "ed2k://|file|å½äº¤é©¬å°ç²¾çå¤æé©¬é½å¹²é³ç¿äºAline & Horseè¯±æ å¤§é å­¦ç-æ¼äº® çµå½± åäº¬ éåºéå¥¸ Açå·æ-å·çª¥-è½®å¥¸-å¼ºå¥¸-å°çµå½±-äººå¦,ç|840683520|F54CC78851C66118EA674205B126A7DE|/";

  memcpy(param.url, source.c_str(), sizeof(param.url));
  memcpy(param.save_path, "F:\\", sizeof(param.save_path));
  param.type = kEd2k;
  //bool ret = download_mgr->AddDownload(param);
  DownloadInfoStruct download_struct;
  download_struct.download_param = param;
  bool ret = download_mgr->Init(download_struct);
  //std::cout << "add ed2k download " << (ret?"success":"failed") << std::endl;
  Sleep(5000);
  double progress = 0.0;
  while ((100 - progress) > 0.1) {
    DownloadProcessInfo info;
    download_mgr->GetDownloadProcessInfo(1, &info);
    std::cout << "id = " << 1 << ", "
              << "file_name = " << info.file_name << ", "
              << "file_size = " << info.file_size << ", "
              << "progress = " << info.progress << ", "
              << "speed = " << info.speed << ", "
              << "status = " << info.status << std::endl;

    progress = info.progress;

    if (progress > 50) {
      download_mgr->PauseTask(1);
      break;
    }
  }
  return ret;
}

int InitQbit(int argc, char* argv[]) {
  HMODULE handle = LoadLibraryA("qbittorrent-nox.dll");
  if (handle != NULL) {
    PFNInit p_init = (PFNInit)GetProcAddress(handle, "Init");
    if (!p_init) {
      std::cout << "get p_init func failed!" << std::endl;
      return 0;
    }
    return p_init(argc, argv);
  }
  return 0;
}


IDownload* GetDownloadMgrQbit() {
  HMODULE handle = LoadLibraryA("qbittorrent-nox.dll");
  if (handle != NULL) {
    PFNGetDownloadMgr mgr =
        (PFNGetDownloadMgr)GetProcAddress(handle, "GetDownloadMgr");
    if (!mgr) {
      std::cout << "get download mgr func failed!" << std::endl;
      return nullptr;
    }
    return mgr();
  }
  return nullptr;
}

int DownloadQbit() {
  Sleep(8000);
  IDownload* download_mgr = GetDownloadMgrQbit();
  std::cout << "add download" << std::endl;
  DownloadParam param;
  param.id = 2;
  std::string source = "C:\\Users\\Administrator\\Downloads\\c16e8276eac0d64ddac48f7f15925b2193184ac9.torrent";
  memcpy(param.url, source.c_str(), sizeof(param.url));
  memcpy(param.save_path, "F:\\", sizeof(param.save_path));
  param.type = kBt_file;


  bool ret = download_mgr->AddDownload(param);
  std::cout << "add download success, ret is " << ret << std::endl;
  double progress = 0.0;
  while ((100 - progress) > 0.1) {
    DownloadProcessInfo info;
    download_mgr->GetDownloadProcessInfo(2, &info);
    std::cout << "id = " << 2 << ", "
              << "file_name = " << info.file_name << ", "
              << "file_size = " << info.file_size << ", "
              << "progress = " << info.progress << ", "
              << "speed = " << info.speed << ", "
              << "status = " << info.status 
              << std::endl;
              
    progress = info.progress;

    if (progress > 50) {
      download_mgr->PauseTask(1);
      break;
    }
  }
  return ret;
}



int main(int argc, char* argv[]) {
  std::future<int> fut = std::async(std::bind(Init, argc, argv));
  //std::future<int> fut_qbit = std::async(std::bind(InitQbit, argc, argv));
  std::future<int> fut_download = std::async(Download);
  //std::future<int> fut_download_Qbit = std::async(DownloadQbit);


  //std::chrono::milliseconds span(5000);
  //if (fut.wait_for(span) == std::future_status::timeout) {
  //  UnInit();
  //}


  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return 0;
}