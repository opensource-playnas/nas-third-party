#ifndef DownloadMgr_h__
#define DownloadMgr_h__

#include <mutex>
#include "IDownload.h"

typedef std::map<unsigned long, DownloadParam> DownloadParamMap;

struct StatisticInfo {
  StatisticInfo() {
    download_bytes = 0.0;
    upload_bytes = 0.0;
  }
  double download_bytes;
  double upload_bytes;
};

class DownloadMgr : public IDownload{
 protected:
  DownloadMgr();
  virtual ~DownloadMgr();

 private:
  DownloadMgr(const DownloadMgr&);
  DownloadMgr& operator=(const DownloadMgr&);

 public:
  static DownloadMgr* Instance();
  virtual bool Init(const DownloadInfoStruct& info);
  virtual DownloadErrorCode AddDownload(const DownloadParam& param);
  virtual bool GetDownloadProcessInfo(unsigned long id, DownloadProcessInfo* info);

	virtual bool CancelTask(unsigned long id);
  virtual bool PauseTask(unsigned long id);
  virtual bool ResumeTask(unsigned long id);
  virtual bool GetDownloadStatisticsInfo(DownloadStatisticsInfo* statistic_info);
  virtual int GetTorrentFileCount(const DownloadParam* param);
  virtual TorrentFileInfo GetTorrentFileInfoByIndex(const DownloadParam* param, int index);
  virtual DownloadErrorCode AddDownload(const DownloadParam* param, int count, TorrentFileInfo* file_info);

 private:
	bool WriteEd2kFile(const char* url);
  CED2KLink* GetEd2kLink(const wxString& url);
	CPartFile* GetPartFile(unsigned long id);
  bool MoveFile(int id, const char* save_path);
	std::string GetSavePath(unsigned long id);
	bool IsExistedEd2kLink(const char* url);
	bool RemoveFile(int id, const char* save_path);
	DownloadStatus GetDownloadStatus(const CPartFile* file);
  bool IsValid(const char * url);

 private:
  std::mutex mutex_;
  DownloadParamMap download_param_;
  std::map<unsigned long, StatisticInfo> task_statistic_info_;
};

#endif  // DownloadMgr_h__
