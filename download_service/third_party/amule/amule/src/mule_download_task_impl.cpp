#include <algorithm>
#include <filesystem>
#include <string>

#include <common/Format.h>  // Needed for CFormat
#include <wx/app.h>
#include <wx/textfile.h>
#include "Logger.h"
#include "amule.h"
#include "DownloadQueue.h"
#include "ScopedPtr.h"
#include "mule_download_task_impl.h"

MuleDownloadTaskImpl::MuleDownloadTaskImpl() {}
MuleDownloadTaskImpl::~MuleDownloadTaskImpl() {}

void MuleDownloadTaskImpl::SetParam(const DownloadParam& param) {
  param_ = param;
  memcpy(file_base_info_.file_name, param.file_name, sizeof(param.file_name));
  file_link_ = GetED2KFileLink();
  tmp_file_path_ = GetTempFilePath();
}

DownloadErrorCode MuleDownloadTaskImpl::Start() {
  DownloadErrorCode ret = DownloadErrorCode::kNormal;

  do {
    ret = CheckCondition(param_.url);
    if (ret != DownloadErrorCode::kNormal) {
      break;
    }

    ret = CreateTempFile();
    if (ret != DownloadErrorCode::kNormal) {
      break;
    }

    if (!WriteEd2kFile(param_.url)) {
      AddLogLineC(CFormat(_("Failed to write ED2KLinks, url = '%s'")) %
                  (void*)param_.url);
      ret = DownloadErrorCode::kWriteEd2kFileFailed;
      break;
    }

    try {
      theApp->AddLinksFromFile();
    } catch (const wxString& err) {
      AddLogLineC(CFormat(_("add link from file ERROR: %s")) % err);
    } catch (...) {
      AddLogLineC(CFormat(_("add link from file ERROR")));
    }

    is_started_ = true;

  } while (0);

  return ret;
}

DownloadErrorCode MuleDownloadTaskImpl::CheckCondition(const char* source) {
  DownloadErrorCode ret = DownloadErrorCode::kNormal;

  do {
    if (!LinkIsValid(source)) {
      AddLogLineC(wxT("Invalid ed2k link!"));
      ret = DownloadErrorCode::kEd2kLinkInvalid;
      break;
    }

    //if (IsExistedEd2kLink()) {
    //  AddLogLineC(CFormat(_("download param has existed url = '%s'")) %
    //              (void*)source);
    //  ret = DownloadErrorCode::kEd2kTaskExisted;
    //  break;
    //}

  } while (0);

  return ret;
}

bool MuleDownloadTaskImpl::Pause() {
  if (CPartFile* part_file = GetPartFile()) {
    part_file->PauseFile();
  }

  return true;
}

bool MuleDownloadTaskImpl::Resume(bool is_failed) {
  if (is_failed) {
    Cancel(false);
    Start();
    cancel_ = false;
  } else if (CPartFile* part_file = GetPartFile()) {
    part_file->ResumeFile();
  }

  return true;
}

bool MuleDownloadTaskImpl::Cancel(bool is_delete_local_file) {
  bool ret = false;
  do {
    cancel_ = true;

    if (tmp_file_.IsOpened()) {
      tmp_file_.Close();
    }

    if (is_delete_local_file) {
      if (wxFile::Exists(tmp_file_path_) && !wxRemoveFile(tmp_file_path_)) {
        break;
      }
      if (wxFile::Exists(final_file_path_) && !wxRemoveFile(final_file_path_)) {
        break;
      }
    }
    DeletePartFile();

    ret = true;

  } while (0);

  return ret;
}

bool MuleDownloadTaskImpl::GetProgressInfo(DownloadProgressInfo* info) {
  bool ret = false;

  do {
    if (!info) {
      break;
    }
    CPartFile* part_file = GetPartFile();
    if (cancel_ || !part_file)
      break;

    wxString save_path_utf8(param_.save_path, wxConvUTF8);
    CPath save_path(save_path_utf8);
    uint64 free = CPath::GetFreeSpaceAt(save_path);
    if (free < part_file->GetFileSize()) {
      info->err_code = DownloadErrorCode::kPerformFailed;
      Pause();
      break;
    }

    wxString test = part_file->GetFullName().GetRaw();
    wxString test1 = part_file->GetFullName().GetFullName().GetRaw();
    wxString test2 = part_file->GetFilePath().GetRaw();

    wxString name = part_file->GetFileName().GetRaw();
    const wxCharBuffer data = part_file->GetFileName().GetRaw().utf8_str();
    std::string file_name(data.data(), data.length());
    memcpy(info->file_name, file_name.c_str(),
             file_name.size());
    // info->file_size = file->GetFileSize();

    uint16 hashingProgress = part_file->GetHashingProgress();
    double percent = hashingProgress == 0 ? part_file->GetPercentCompleted()
                                          : 100.0 * hashingProgress * PARTSIZE /
                                                part_file->GetFileSize();
    if (part_file->IsCompleted()) {
      percent = 100.0;
      // 下载完成，再将文件拷贝到指定保存路径下
      RenameTempFile();
    } else if (percent > 99.9) {
      percent = 99.9;
    }

    info->total_progress = percent;
    info->download_speed = part_file->GetKBpsDown() * 1024;  // 单位：bytes/s
    uint8 time = thePrefs::GetStatsAverageMinutes() * 60;
    // info->upload_speed = file->statistic.GetAllTimeTransferred() / time /
    // 1024.0;
    // info->upload_speed = theStats::GetUploadRate();  // bytes/s
    info->upload_speed = 0;

    if (part_file->GetStatus() == PS_ERROR) {
      info->err_code = DownloadErrorCode::kPerformFailed;
    }

    ret = true;

  } while (0);

  return ret;
}

bool MuleDownloadTaskImpl::ParseLink(const char* link, ParseResult* result) {
  if (!file_link_ || !result) {
    return false;
  }
  const wxCharBuffer data = file_link_->GetName().utf8_str();
  std::string file_name(data.data(), data.length());

  memcpy(result->file_info.file_name, file_name.c_str(), file_name.size());
  result->file_info.file_size = file_link_->GetSize();

  return true;
}

bool MuleDownloadTaskImpl::Valid(bool is_finished) {
  if (is_finished) {
    if (final_file_path_.empty()) {
      std::string file_path = param_.save_path;
      file_path.append("/");
      file_path.append(file_base_info_.file_name);
      final_file_path_ = wxString(file_path.c_str(), wxConvUTF8);
    }

    if (!wxFile::Exists(final_file_path_)) {
      return false;
    }
  } else {
    if (tmp_file_path_.empty()) {
      tmp_file_path_ = GetTempFilePath();
    }
    if (!wxFile::Exists(tmp_file_path_)) {
      return false;
    }
  }

  return true;
}

const char* MuleDownloadTaskImpl::GetTaskHash() {
  if (!file_link_) {
    return NULL;
  }
  const unsigned char * hash = file_link_->GetHashKey().GetHash();
  int length = strlen((const char*)(hash));
  char* buf = new char[length + 1];
  memset(buf, 0, length);
  memcpy(buf, hash, length);
  buf[length] = '\0';
  return buf;
}

void MuleDownloadTaskImpl::ReleaseBuf(const char* buf) {
  if (buf) {
    delete[] buf;
  }
}

bool MuleDownloadTaskImpl::LinkIsValid(const char* url) {
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

bool MuleDownloadTaskImpl::IsExistedEd2kLink() {
  if (!file_link_) {
    return false;
  }

  return theApp->downloadqueue->IsFileExisting(file_link_->GetHashKey());
}

CED2KLink* MuleDownloadTaskImpl::GetEd2kLink(const wxString& url) {
  CED2KLink* link = NULL;
  if (url.compare(0, 7, wxT("ed2k://")) != 0) {
    AddLogLineC(CFormat(_("Unknown protocol of link: %s")) % url);
    return link;
  }

  wxASSERT(!url.IsEmpty());
  wxString URI = url;

  // Need the links to end with /, otherwise CreateLinkFromUrl crashes us.
  if (URI.Last() != wxT('/')) {
    URI += wxT("/");
  }
  try {
    link = CED2KLink::CreateLinkFromUrl(URI);
  } catch (const wxString& err) {
    AddLogLineC(CFormat(_("Invalid eD2k link! ERROR: %s")) % err);
    link = NULL;
  }
  return link;
}

bool MuleDownloadTaskImpl::WriteEd2kFile(const char* url) {
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

bool MuleDownloadTaskImpl::MoveFile(const char* save_path) {
  CPartFile* file = GetPartFile();
  if (!file)
    return false;

  wxString save_path_utf8(save_path, wxConvUTF8);
  save_path_utf8 += "/";

  wxString index;
  index = CFormat(wxT("%03d")) % file->GetPartMetNumber();
  CPath config_temp = thePrefs::GetTempDir();
  wxString source_temp = config_temp.GetRaw() + "/";
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
    wxString source_path =
        file->GetFilePath().JoinPaths(file->GetFileName()).GetRaw();
    wxString save_file_path = save_path_utf8 + file->GetFileName().GetRaw();
    wxRenameFile(source_path, save_file_path);
  }

  return false;
}

DownloadErrorCode MuleDownloadTaskImpl::CreateTempFile() {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  do {
    if (!file_link_) {
      err = DownloadErrorCode::kCreateTaskFailed;
      break;
    }

    tmp_file_path_ = GetTempFilePath();
    if (!wxFile::Exists(tmp_file_path_)) {
      if (!tmp_file_.Create(tmp_file_path_, true)) {
        err = DownloadErrorCode::kCreateFileFailed;
        break;
      }
    }
    if (!tmp_file_.Open(tmp_file_path_, wxFile::OpenMode::write)) {
      err = DownloadErrorCode::kCreateFileFailed;
      break;
    }

    // 将文件指针移动到所需的大小位置
    uint64 file_size = file_link_->GetSize();
    if (tmp_file_.SeekEnd(file_size - 1) == wxInvalidOffset) {
      // 移动文件指针失败，关闭文件并删除它
      tmp_file_.Close();
      wxRemoveFile(tmp_file_path_);
      err = DownloadErrorCode::kCreateFileFailed;
      break;
    }

    // 写入一个字节，使文件达到固定大小
    if (tmp_file_.Write("\0", 1) != 1) {
      // 写入失败，关闭文件并删除它
      tmp_file_.Close();
      wxRemoveFile(tmp_file_path_);
      err = DownloadErrorCode::kCreateFileFailed;
      break;
    }

  } while (0);

  return err;
}

CPartFile* MuleDownloadTaskImpl::GetPartFile() {
  CPartFile* part_file = nullptr;

  if (!file_link_) {
    AddLogLineCS(wxT("get ed2k file link failed!"));
    return part_file;
  }

  if (theApp->downloadqueue->IsFileExisting(file_link_->GetHashKey())) {
    // Must be a shared file if we are to add hashes or sources
    part_file = theApp->downloadqueue->GetFileByID(file_link_->GetHashKey());
  }

  return part_file;
}

CED2KFileLink* MuleDownloadTaskImpl::GetED2KFileLink() {
  wxString utf8_str(param_.url, wxConvUTF8);
  // CScopedPtr<CED2KLink> link = GetEd2kLink(utf8_str);
  CED2KLink* link = GetEd2kLink(utf8_str);
  CED2KFileLink* ed2k_file_link = dynamic_cast<CED2KFileLink*>(link);
  return ed2k_file_link;
}

wxString MuleDownloadTaskImpl::GetTempFilePath() {
  wxString file_path;
  wxString wx_file_name;

  wxString save_path_utf8(param_.save_path, wxConvUTF8);
  save_path_utf8 += "/";

  std::string file_name = file_base_info_.file_name;
  if (file_name.empty()) {
    if (!file_link_) {
      return file_path;
    }
    wx_file_name = file_link_->GetName();
  } else {
    wx_file_name = wxString(file_name.c_str(), wxConvUTF8);
  }

  file_path = save_path_utf8 + wx_file_name + "." + param_.id + ".nas.tmp";
  return file_path;
}

void MuleDownloadTaskImpl::RenameTempFile() {
  do {
    CPartFile* file = GetPartFile();
    if (!file)
      break;

    wxString save_path_utf8(param_.save_path, wxConvUTF8);
    save_path_utf8 += "/";

    wxString source_path =
        file->GetFilePath().JoinPaths(file->GetFileName()).GetRaw();
    wxString save_file_path = save_path_utf8 + file->GetFileName().GetRaw();
    if (!wxRenameFile(source_path, save_file_path)) {
      download_progress_info_.err_code =
          DownloadErrorCode::kRenameTempFileFailed;
      break;
    }

    tmp_file_.Close();
    if (!wxRemoveFile(tmp_file_path_)) {
      download_progress_info_.err_code =
          DownloadErrorCode::kRenameTempFileFailed;
      break;
    }

    final_file_path_ = save_file_path;
  } while (0);
}

void MuleDownloadTaskImpl::DeletePartFile() {
  if (CPartFile* part_file = GetPartFile()) {
    if (part_file->IsCompleted()) {
      ListOfUInts32 toClear;
      toClear.push_back(part_file->ECID());
      theApp->downloadqueue->ClearCompleted(toClear);
    }
    part_file->Delete();
  }
}