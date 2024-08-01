#ifndef IDownload_h__
#define IDownload_h__

enum DownloadType {
  kUnknown = 0,
  kEd2k = 1,
  kBt_file = 2,
  kMagnet = 3,
  kHttp = 4
};

enum DownloadStatus {
  kDownloading = 0,
  kPauseDownload = 1,
  kFinished = 2,
  kDelete = 3
};

enum DownloadErrorCode {
  kNormal = 0,
  kExisted = 1,
  kUrlInvalid = 2,
  kBtFileContentInvalid = 3,
  kEd2kLinkInvalid = 4,
  kCreateTaskFailed = 5,
  kTaskIdNotExist = 6
};

enum DownloadItemSelect
{
    kIgnored = 0,
    kNormalDownload = 1,
    kHigh = 6,
    kMaximum = 7,
    kMixed = -1
};

struct DownloadStatisticsInfo {
  DownloadStatisticsInfo() {
    download_rate = 0.0;
    upload_rate = 0.0;
    accumulate_download = 0.0;
    accumulate_upload = 0.0;
  }
  double download_rate;
  double upload_rate;
  double accumulate_download;
  double accumulate_upload;
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
    file_size = 0;
    progress = 0.0;
    speed = 0.0;
    upload_speed = 0.0;
    status = kDownloading;
    create_time = 0;
    finish_or_delete_time = 0;
    err_code = kNormal;
  }
  char file_name[1024];
  unsigned long long file_size;
  double progress;
  double speed;
  double upload_speed;
  DownloadStatus status;
  double create_time;
  double finish_or_delete_time;
  DownloadErrorCode err_code;
};

struct DownloadInfoStruct {
  DownloadParam download_param;
  DownloadProcessInfo download_info;
};

struct TorrentFileInfo {
    TorrentFileInfo() {
        memset(file_name, 0, sizeof(file_name));
        memset(file_type, 0, sizeof(file_type));
        file_size = 0;
        item_select = kNormalDownload;
    }
    char file_name[1024];
    unsigned long long file_size;
    char file_type[32];
    DownloadItemSelect item_select;
};

class IDownload {
 public:
  virtual ~IDownload() = default;
  virtual bool Init(const DownloadInfoStruct& info) = 0;
  virtual DownloadErrorCode AddDownload(const DownloadParam& param) = 0;
  virtual bool GetDownloadProcessInfo(unsigned long id,
                                      DownloadProcessInfo* info) = 0;
  virtual bool PauseTask(unsigned long id) = 0;
  virtual bool ResumeTask(unsigned long id) = 0;
  virtual bool CancelTask(unsigned long id) = 0;
  virtual bool GetDownloadStatisticsInfo(DownloadStatisticsInfo* statistic_info) = 0;
  virtual int GetTorrentFileCount(const DownloadParam * param) = 0;
  virtual TorrentFileInfo GetTorrentFileInfoByIndex(const DownloadParam * param, int index) = 0;
  virtual DownloadErrorCode AddDownload(const DownloadParam * param, int count, TorrentFileInfo * file_info) = 0;
};

#endif  // IDownload_h__
