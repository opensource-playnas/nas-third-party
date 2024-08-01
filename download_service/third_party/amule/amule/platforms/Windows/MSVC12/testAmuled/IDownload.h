#ifndef IDownload_h__
#define IDownload_h__

#include <memory>

enum DownloadType {
  kUnknown = 0,
  kEd2k = 1,
  kBt_file = 2,
  kMagnet = 3,
  kHttp = 4
};

enum DownloadStatus {
  KDownloading = 0,
  KPauseDownload = 1,
  kFinished = 2,
  kDelete = 3
};

struct DownloadParam {
  DownloadParam() {
    id = 0;
    memset(url, 0, sizeof(url));
    memset(save_path, 0, sizeof(save_path));
    type = kUnknown;
  }

  unsigned long id;
  char url[2048];
  char save_path[2048];
  DownloadType type;
};

struct DownloadProcessInfo {
  DownloadProcessInfo() {
    memset(file_name, 0, sizeof(file_name));
    file_size = 0.0;
    progress = 0.0;
    speed = 0.0;
    status = KDownloading;
  }
  char file_name[1024];
  unsigned long long file_size;
  double progress;
  double speed;
  DownloadStatus status;
};

struct DownloadInfoStruct {
  DownloadParam download_param;
  DownloadProcessInfo download_info;
};

class IDownload {
 public:
  virtual ~IDownload() = default;
  virtual bool Init(const DownloadInfoStruct& info) = 0;
  virtual bool AddDownload(const DownloadParam& param) = 0;
  virtual bool GetDownloadProcessInfo(unsigned long id,
                                      DownloadProcessInfo* info) = 0;
  virtual bool PauseTask(unsigned long id) = 0;
  virtual bool ResumeTask(unsigned long id) = 0;
  virtual bool CancelTask(unsigned long id) = 0;
};

#endif  // IDownload_h__
