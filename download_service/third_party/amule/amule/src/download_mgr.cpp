#include <filesystem>
#include <algorithm>
#include <string>


#include <wx/textfile.h>
#include <wx/app.h>
#include <common/Format.h>	// Needed for CFormat
#include "ED2KLink.h"
#include "amule.h"

#include "download_mgr.h"

DownloadMgr::DownloadMgr(){}

DownloadMgr::~DownloadMgr() {}

DownloadMgr* DownloadMgr::Instance() {
  static DownloadMgr obj;
  return &obj;
}

bool DownloadMgr::Init(const DownloadInfoStruct& info) {
  std::unique_lock<std::mutex> lock(mutex_);
  download_param_[info.download_param.id] = info.download_param;
  return true;
}

DownloadErrorCode DownloadMgr::AddDownload(const DownloadParam& param) {
  DownloadErrorCode ret = kNormal;

  do {
    std::unique_lock<std::mutex> lock(mutex_);
    if (download_param_.find(param.id) != download_param_.end()) {
      AddLogLineC(CFormat( _("download param has existed id = '%d'") ) % param.id);
      ret = kExisted;
      break;
    }

    if (!IsValid(param.url)) {
      AddLogLineC(wxT("Invalid ed2k link!"));
      ret = kUrlInvalid;
      break;
    }

    std::string url = param.url;

    //test
    //static int i = 0;
    //if (i == 1) {
    //  wxString utf8_url(param.url, wxConvUTF8);
    //  CScopedPtr<CED2KLink> link = GetEd2kLink(utf8_url);
    //  CED2KFileLink* ed2k_file_link = dynamic_cast<CED2KFileLink*>(link.get());
    //  CKnownFile* file = theApp->sharedfiles->GetFileByID(ed2k_file_link->GetHashKey());
    //  wxString save_path_utf8(param.save_path, wxConvUTF8);
    //  save_path_utf8 += "\\";
    //  wxString save_file_path = save_path_utf8 + file->GetFileName().GetRaw();
    //  //wxString test = file->GetFullName().GetRaw();
    //  //wxString test1 = file->GetFullName().GetFullName().GetRaw();
    //  wxString test2 = file->GetFilePath().GetRaw();
    //  wxString path = file->GetFilePath().JoinPaths(file->GetFileName()).GetRaw();

    //  wxString name = file->GetFileName().GetRaw();
    //  const wxCharBuffer data = file->GetFileName().GetRaw().utf8_str();
    //  std::string file_name = data;
    //  wxRenameFile(path, save_file_path);
    //  wxString index;
    //  int a = 003;
    //  index = CFormat(wxT("%03d")) % a;
    //  wxString save_part = save_path_utf8 + index + wxT(".part");
    //  wxString save_part_met = save_path_utf8 + index + wxT(".part.met");
    //  wxString save_part_met_bak =
    //  save_path_utf8 + index + wxT(".part.met.bak");

    //  wxRemoveFile("001.part");
    //  wxRemoveFile(save_part_met);
    //  wxRemoveFile(save_part_met_bak);
    //}
    //++i;

    if (IsExistedEd2kLink(param.url)) {
      AddLogLineC(CFormat(_("download param has existed url = '%s'")) % url);
      ret = kExisted;
      break;
    }

    if (!WriteEd2kFile(param.url)) {
      AddLogLineC(CFormat(_("Failed to write ED2KLinks, url = '%s'")) % url);
      ret = kCreateTaskFailed;
      break;
    }

    download_param_[param.id] = param;
    theApp->AddLinksFromFile();

    //std::shared_ptr<std::thread> thr = std::make_shared<std::thread>(
    //    std::bind(&DownloadMgr::MoveFile, this, param.id, param.save_path));
    MoveFile(param.id, param.save_path);
  } while (0);

  return ret;
}

bool DownloadMgr::GetDownloadProcessInfo(unsigned long id, DownloadProcessInfo* info) {
  bool ret = false;

  do 
  {
    if (!info) {
      break;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    CPartFile* file = GetPartFile(id);
    if (!file) break;

    wxString test = file->GetFullName().GetRaw();
    wxString test1 = file->GetFullName().GetFullName().GetRaw();
    wxString test2 = file->GetFilePath().GetRaw();

    wxString name = file->GetFileName().GetRaw();
    const wxCharBuffer data = file->GetFileName().GetRaw().utf8_str();
    std::string file_name = data;
    memcpy_s(info->file_name, sizeof(info->file_name), file_name.c_str(), file_name.size());
    info->file_size = file->GetFileSize();

    uint16 hashingProgress = file->GetHashingProgress();
    double percent = hashingProgress == 0 ? file->GetPercentCompleted()
                                          : 100.0 * hashingProgress * PARTSIZE /
                                                file->GetFileSize();
    if (file->IsCompleted()) {
      percent = 100.0;
      info->status = kFinished;
      info->speed = 0;
      //下载完成，再将文件拷贝到指定保存路径下
      std::string save_path = GetSavePath(id);
      MoveFile(id, save_path.c_str());
    } else if (percent > 99.9) {
      percent = 99.9;
    }

    info->progress = percent;
    info->speed = file->GetKBpsDown();
    uint8 time = thePrefs::GetStatsAverageMinutes() * 60;
    //info->upload_speed = file->statistic.GetAllTimeTransferred() / time / 1024.0;
    info->upload_speed = theStats::GetUploadRate() / 1024.0; //kb/s

    ret = true;

  } while (0);

  return ret;
}

bool DownloadMgr::CancelTask(unsigned long id) {
  bool ret = false;

  do {
    std::unique_lock<std::mutex> lock(mutex_);
    CPartFile* file = GetPartFile(id);
    if (!file) break;
    auto iter = download_param_.find(id);
    if (iter != download_param_.end()) {
      RemoveFile(id, GetSavePath(id).c_str());
      download_param_.erase(id);
      file->Delete();
    }
    ret = true;
  } while (0);

  return ret;
}

bool DownloadMgr::PauseTask(unsigned long id) { 
  bool ret = false;

  do {
      std::unique_lock<std::mutex> lock(mutex_);
      CPartFile* file = GetPartFile(id);
      if (!file) break;
      file->PauseFile();
      ret = true;
  } while (0);

  return ret;
}

bool DownloadMgr::ResumeTask(unsigned long id) {
  bool ret = false;

  do {
      std::unique_lock<std::mutex> lock(mutex_);
      CPartFile* file = GetPartFile(id);
      if (!file) break;
      file->ResumeFile();
      ret = true;
  } while (0);

  return ret;
}

bool DownloadMgr::GetDownloadStatisticsInfo(DownloadStatisticsInfo* statistic_info) {
  bool ret = false;

  do {
    if (!statistic_info) {
      break;
    }

    statistic_info->accumulate_upload = theStats::GetTotalSentBytes()/ 1024.0 / 1024.0 / 1024.0;         //GB

    //statistic_info->accumulate_download = theStats::GetTotalReceivedBytes() / 1024.0 / 1024.0 / 1024.0;  //GB

    //std::unique_lock<std::mutex> lock(mutex_);
    //for (auto iter = task_statistic_info_.begin(); iter != task_statistic_info_.end(); ++iter) {
    //  statistic_info->accumulate_download += iter->second.download_bytes / 1024.0 / 1024.0 / 1024.0;  // GB
    //}

    //int size = download_param_.size();
    //if (size <= 0) {
    //  break;
    //}

    //statistic_info->download_rate = theStats::GetDownloadRate() / 1024.0;  //KB/s
    //statistic_info->upload_rate = theStats::GetUploadRate() / 1024.0;     //KB/s

    ret = true;
  } while (0);

  return ret;
}

DownloadErrorCode DownloadMgr::AddDownload(const DownloadParam* param,
                                           int count,
                                           TorrentFileInfo* file_info) {
  return DownloadErrorCode();
}

int DownloadMgr::GetTorrentFileCount(const DownloadParam* param) { return 0; }

TorrentFileInfo DownloadMgr::GetTorrentFileInfoByIndex(
    const DownloadParam* param, int index) {
  return TorrentFileInfo();
}

bool DownloadMgr::WriteEd2kFile(const char * url) {
  wxTextFile ed2kFile(thePrefs::GetConfigDir() + wxT("ED2KLinks"));
  if (!ed2kFile.Exists()) {
    ed2kFile.Create();
  }
  if (ed2kFile.Open()) {
    wxString link;
    wxString utf8_str(url, wxConvUTF8);
    if (theApp->CheckPassedLink(utf8_str, link, 0)) {
      ed2kFile.AddLine(link);
      ed2kFile.Write();
      return true;
    }
  } else {
    AddLogLineCS(wxT("Failed to open 'ED2KLinks', cannot add links."));
  }
  return false;
 }

CED2KLink* DownloadMgr::GetEd2kLink(const wxString& url) {
  if (url.compare(0, 7, wxT("ed2k://")) != 0) {
    AddLogLineC(CFormat(_("Unknown protocol of link: %s")) % url);
    return NULL;
  }

  wxASSERT(!url.IsEmpty());
  wxString URI = url;

   // Need the links to end with /, otherwise CreateLinkFromUrl crashes us.
   if (URI.Last() != wxT('/')) {
     URI += wxT("/");
   }
   return CED2KLink::CreateLinkFromUrl(URI);
}

CPartFile* DownloadMgr::GetPartFile(unsigned long id) {
  CPartFile* file = NULL;
  auto iter = download_param_.find(id);
  if (iter != download_param_.end()) {
    wxString utf8_str(iter->second.url, wxConvUTF8);
    CScopedPtr<CED2KLink> link = GetEd2kLink(utf8_str);
    CED2KFileLink* ed2k_file_link = dynamic_cast<CED2KFileLink*>(link.get());

    if (!ed2k_file_link) {
      AddLogLineCS(wxT("get ed2k file link failed!"));
      return file;
    }

    if (theApp->downloadqueue->IsFileExisting(ed2k_file_link->GetHashKey())) {
        // Must be a shared file if we are to add hashes or sources
        file = theApp->downloadqueue->GetFileByID(ed2k_file_link->GetHashKey());
      }
  }
  return file;
}

bool DownloadMgr::MoveFile(int id, const char* save_path) {
  CPartFile* file = GetPartFile(id);
  if (!file) return false;

  wxString save_path_utf8(save_path, wxConvUTF8);
  save_path_utf8 += "\\";

  wxString index;
  index = CFormat(wxT("%03d")) % file->GetPartMetNumber();
  CPath config_temp = thePrefs::GetTempDir();
  wxString source_temp = config_temp.GetRaw() + "\\";
  wxString part = source_temp + index + wxT(".part");
  wxString part_met = source_temp + index + wxT(".part.met");
  wxString part_met_bak = source_temp + index + wxT(".part.met.bak");
  wxString save_part = save_path_utf8 + index + wxT(".part");
  wxString save_part_met = save_path_utf8 + index + wxT(".part.met");
  wxString save_part_met_bak = save_path_utf8 + index + wxT(".part.met.bak");

  if (file->IsPartFile() && !file->IsCompleted()) {
    if (wxCopyFile(part, save_part) && wxCopyFile(part_met, save_part_met) &&
        wxCopyFile(part_met_bak, save_part_met_bak)) {
      return true;
    }
  } else {
    wxRemoveFile(save_part);
    wxRemoveFile(save_part_met);
    wxRemoveFile(save_part_met_bak);
    wxString source_path = file->GetFilePath().JoinPaths(file->GetFileName()).GetRaw();
    wxString save_file_path = save_path_utf8 + file->GetFileName().GetRaw();
    wxRenameFile(source_path, save_file_path);
  }

  return false;
}

std::string DownloadMgr::GetSavePath(unsigned long id) {
  std::string save_path;
  auto iter = download_param_.find(id);
  if (iter != download_param_.end()) {
    save_path = iter->second.save_path;
  }
  return save_path;
}

bool DownloadMgr::IsExistedEd2kLink(const char* url) {
  wxString utf8_str(url, wxConvUTF8);
  CScopedPtr<CED2KLink> link = GetEd2kLink(utf8_str);
  CED2KFileLink* ed2k_file_link = dynamic_cast<CED2KFileLink*>(link.get());

  if (!ed2k_file_link) {
    return false;
  }

  return theApp->downloadqueue->IsFileExisting(ed2k_file_link->GetHashKey());
}

bool DownloadMgr::RemoveFile(int id, const char* save_path) {
  CPartFile* file = GetPartFile(id);
  if (!file) return false;

  wxString index;
  index = CFormat(wxT("%03d")) % file->GetPartMetNumber();
  wxString save_path_utf8(save_path, wxConvUTF8);
  save_path_utf8 += "\\";

  wxString save_part = save_path_utf8 + index + wxT(".part");
  wxString save_part_met = save_path_utf8 + index + wxT(".part.met");
  wxString save_part_met_bak = save_path_utf8 + index + wxT(".part.met.bak");
  wxRemoveFile(save_part);
  wxRemoveFile(save_part_met);
  wxRemoveFile(save_part_met_bak);
  return true;

}

DownloadStatus DownloadMgr::GetDownloadStatus(const CPartFile* file) {
  DownloadStatus status = kDownloading;
  if (!file) {
    return status;
  }

  if (file->IsPaused() || file->IsStopped()) {
    status = kPauseDownload;
  } else if (file->IsCompleted()) {
    status = kFinished;
  }
  return status;
}

bool DownloadMgr::IsValid(const char* url) {
  bool ret = false;
  do {
      if (!url || url[0] == '\0') {
         break;
      }
      wxString utf8_str(url, wxConvUTF8);
      wxASSERT(!utf8_str.IsEmpty());
      if (utf8_str.compare(0, 7, wxT("ed2k://")) != 0) {
        AddLogLineC(CFormat(_("Unknown protocol of link: %s")) % utf8_str);
        break;
      }

      wxString URI = utf8_str;

      // Need the links to end with /, otherwise CreateLinkFromUrl crashes us.
      if (URI.Last() != wxT('/')) {
        URI += wxT("/");
      }

      try {
        CScopedPtr<CED2KLink> uri(CED2KLink::CreateLinkFromUrl(URI));
        if (uri.get()) {
          ret = true;
        }
      } catch (const wxString& err) {
        AddLogLineC(CFormat(_("Invalid eD2k link! ERROR: %s")) % err);
      }

  } while (0);

  return ret;
}
