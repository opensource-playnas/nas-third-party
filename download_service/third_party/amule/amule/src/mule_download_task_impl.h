#ifndef NAS_AMULE_ROOT_AMULE_SRC_MULE_DOWNLOAD_TASK_IMPL_H_
#define NAS_AMULE_ROOT_AMULE_SRC_MULE_DOWNLOAD_TASK_IMPL_H_

#include <string>
#include <mutex>
#include "ED2KLink.h"
#include "PartFile.h"
#include "mule_task_public_define.h"

class MuleDownloadTaskImpl : public MuleDownloadTask {
 public:
  MuleDownloadTaskImpl();
  ~MuleDownloadTaskImpl();
  DownloadErrorCode Start() override;
  DownloadErrorCode CheckCondition(const char* source) override;
  bool Pause() override;
  bool Resume(bool is_failed) override;
  bool Cancel(bool is_delete_local_file) override;
  void SetParam(const DownloadParam& param) override;
  bool GetProgressInfo(DownloadProgressInfo* info) override;
  bool ParseLink(const char* link, ParseResult* result) override;
  bool Valid(bool is_finished) override;
  const char* GetTaskHash() override;
  void ReleaseBuf(const char* buf) override;

private:
  bool LinkIsValid(const char* url);
  bool IsExistedEd2kLink();
  CED2KLink* GetEd2kLink(const wxString& url);
  bool WriteEd2kFile(const char* url);
  bool MoveFile(const char* save_path);
  DownloadErrorCode CreateTempFile();
  CPartFile* GetPartFile();
  CED2KFileLink* GetED2KFileLink();
  wxString GetTempFilePath();
  void RenameTempFile();

  void DeletePartFile();
private:
  DownloadParam param_;
  //CPartFile* part_file_ = NULL;
  CED2KFileLink * file_link_ = NULL;
  FileBaseInfo file_base_info_;
  DownloadProgressInfo download_progress_info_;
  wxFile tmp_file_;
  wxString tmp_file_path_;
  bool cancel_ = false;
  bool is_started_ = false;
  wxString final_file_path_;
};

#endif