#ifndef NAS_THIRD_PARTY_DOWNLOAD_DOWNLOAD_PUBLIC_DEFINE_H_
#define NAS_THIRD_PARTY_DOWNLOAD_DOWNLOAD_PUBLIC_DEFINE_H_

#include <cstring>

static const int kIdLength = 64;
static const int kUrlLength = 4096;
static const int kMaxPath = 260*4;

enum DownloadType {
  kUnknown = 0,  // 未知
  kEd2k = 1,     // ed2k链接
  kTorrent = 2,  // btz种子文件
  kMagnet = 3,   // magnet磁力链接
  kHttp = 4,
  kHttps = 5,
  kFtp = 6,
  kFtps = 7,
  kSftp = 8
};

enum HttpCode
{
  kCont = 100,
  kOk = 200,
  kCreated = 201,
  kAccepted = 202,
  kNoContent = 204,
  kMultipleChoices = 300,
  kMovedPermanently = 301,
  kMovedTemporarily = 302,
  kNotModified = 304,
  kBadRequest = 400,
  kUnauthorized = 401,
  kForbidden = 403,
  kNotFound = 404,
  kRequestTimeout = 408,
  kInternalServerError = 500,
  kNotImplemented = 501,
  kBadGateway = 502,
  kServiceUnavailable = 503,
  kGatewayTimeout = 504
};

enum DownloadErrorCode {
  kNormal = 0,                  // 正常
  kUserIdIsNotExist,        // 用户id不存在
  kTaskIdIsNotExist,// 任务id不存在
  kTaskIdExisted,// 任务id已存在
  kCreateTaskFailed,        // 任务创建失败
  kAddTaskFailed,        // 添加任务失败
  kCreateDirFailed,   // 创建下载目录失败，当前下载目录无法被创建，请尝试下载到其他目录
  kPerformFailed, // 下载过程中失败
  kFileMissing, // 文件丢失
  kExistSameTask, // 存在相同的任务
  kCreateFileFailed, // 创建下载文件失败，当前下载目录无法写入数据，请尝试下载到其他目录
  kRenameTempFileFailed, // 重命名临时文件失败
  kUploadFileFailed, // 上传文件失败
  kFileNameExisted, // 存在相同的任务名称
  kPathExceedLimit, // 路径超出了255字符限制
  kFileIoError,     // 文件已损坏
  kProtectedPath,   // 该下载目录为受保护目录，请尝试下载到其他目录
  kInvalidFileName,   // 无效的文件名称

  // curl
  kCreateCurlTaskFailed = 1000,    // 创建curl任务失败
  kCurlUrlInvalid,              // 链接无效
  kParseCurlUrlFailed,              // 解析curl失败
  kPerformCurlFailed,           // 执行失败
  kPerformCurlStartErrorCode = 1100,    // 执行curl起始错误码
  kCurleUnsupportedProtocol,     // 不支持的协议
  kCurleFailedInit,              // 初始化失败
  kCurleUrlMalformat,            // URL 格式不正确
  kCurleNotBuiltIn,               // 已废弃，不再使用
  kCurleCouldntResolveProxy,     // 无法解析代理服务器
  kCurleCouldntResolveHost,      // 无法解析主机名
  kCurleCouldntConnect,          // 无法连接到主机或代理
  kCurleWeirdServerReply,          // 服务器返回了异常的响应
  kCurleRemoteAccessDenied,        // 远程服务器拒绝访问（例如，登录失败）
  kCurleFtpAcceptFailed,          // FTP 服务器无法接受连接
  kCurleFtpWeirdPassReply,        // FTP 服务器返回了异常的 PASS（密码）响应
  kCurleFtpAcceptTimeout,         // FTP 服务器接受连接超时
  kCurleFtpWeirdPasvReply,       // FTP 服务器返回了异常的 PASV（被动模式）响应
  kCurleFtpWeird227Format,       // FTP 服务器返回了异常的 227 响应（表示被动模式）
  kCurleFtpCantGetHost,          // 无法获取 FTP 主机信息
  kCurleHttp2,                   // HTTP/2 协议层面的问题
  kCurleFtpCouldntSetType,       // 无法设置 FTP 传输类型（例如，ASCII 或二进制）
  kCurlePartialFile,             // 文件只部分传输完成
  kCurleFtpCouldntRetrFile,      // FTP 服务器无法检索文件
  kCurleObsolete20,              // 已废弃，不再使用
  kCurleQuoteError,              // 引用命令失败
  kCurleHttpReturnedError,       // HTTP 服务器返回了错误状态码
  kCurleWriteError,              // 写入本地文件时发生错误
  kCurleObsolete24,              // 已废弃，不再使用
  kCurleUploadFailed,            // 上传文件失败
  kCurleReadError,               // 无法打开或读取文件
  kCurleOutOfMemory,             // 内存不足（如果启用了 `CURL_DOES_CONVERSIONS`，可能表示转换错误）
  kCurleOperationTimedout,       // 操作超时
  kCurleObsolete29,              // 已废弃，不再使用
  kCurleFtpPortFailed,           // FTP PORT 命令失败
  kCurleFtpCouldntUseRest,       // FTP REST 命令失败
  kCurleObsolete32,              // 已废弃，不再使用
  kCurleRangeError,              // RANGE 命令无效
  kCurleHttpPostError,           // HTTP POST 操作失败
  kCurleSslConnectError,         // 使用 SSL 连接时出错
  kCurleBadDownloadResume,       // 无法恢复下载
  kCurleFileCouldntReadFile,     // 无法读取文件
  kCurleLdapCannotBind,          // LDAP 无法绑定
  kCurleLdapSearchFailed,        // LDAP 搜索失败
  kCurleObsolete40,              // 已废弃，不再使用
  kCurleFunctionNotFound,        // 从 7.53.0 版本开始已废弃，不再使用
  kCurleAbortedByCallback,       // 操作被回调函数中止
  kCurleBadFunctionArgument,     // 函数参数错误
  kCurleObsolete44,              // 已废弃，不再使用
  kCurleInterfaceFailed,         // `CURLOPT_INTERFACE` 设置失败
  kCurleObsolete46,              // 已废弃，不再使用
  kCurleTooManyRedirects,        // 重定向次数过多
  kCurleUnknownOption,           // 用户指定了未知选项
  kCurleSetoptOptionSyntax,      // `setopt` 选项语法错误
  kCurleObsolete50,              // 已废弃，不再使用
  kCurleObsolete51,              // 已废弃，不再使用
  kCurleGotNothing,              // 没有收到任何数据
  kCurleSslEngineNotfound,       // 找不到 SSL 加密引擎
  kCurleSslEngineSetfailed,      // 无法将 SSL 加密引擎设置为默认
  kCurleSendError,               // 发送网络数据失败
  kCurleRecvError,               // 接收网络数据失败
  kCurleObsolete57,              // 已废弃，不再使用
  kCurleSslCertproblem,          // 本地证书问题
  kCurleSslCipher,               // 无法使用指定的加密算法
  kCurlePeerFailedVerification,  // 对等方的证书或指纹未通过验证
  kCurleBadContentEncoding,      // 无法识别或错误的内容编码
  kCurleLdapInvalidUrl,          // 无效的 LDAP URL
  kCurleFilesizeExceeded,        // 超过最大文件大小限制
  kCurleUseSslFailed,            // 请求的 FTP SSL 级别失败
  kCurleSendFailRewind,          // 发送数据需要回滚，但回滚失败
  kCurleSslEngineInitfailed,     // 初始化 SSL 引擎失败
  kCurleLoginDenied,             // 用户、密码或类似凭据未被接受，登录失败
  kCurleTftpNotfound,            // 服务器上找不到文件
  kCurleTftpPerm,                // 服务器上的权限问题
  kCurleRemoteDiskFull,          // 服务器磁盘空间不足
  kCurleTftpIllegal,             // 非法的 TFTP 操作
  kCurleTftpUnknownid,           // 未知的传输 ID
  kCurleRemoteFileExists,        // 文件已存在
  kCurleTftpNosuchUser,          // 无此用户
  kCurleConvFailed,              // 转换失败
  kCurleConvReqd,                // 调用者必须使用 `curl_easy_setopt` 选项注册转换回调函数
  kCurleSslCacertBadfile,        // 无法加载 CACERT 文件，文件丢失或格式错误
  kCurleRemoteFileNotFound,      // 未找到远程文件
  kCurleSsh,                     // SSH 层面的错误，具体错误信息需查看错误消息
  kCurleSslShutdownFailed,       // 无法关闭 SSL 连接
  kCurleAgain,                   // 套接字未准备好发送/接收数据，等待准备好后重试
  kCurleSslCrlBadfile,           // 无法加载 CRL 文件，文件丢失或格式错误
  kCurleSslIssuerError,          // 发行人检查失败
  kCurleFtpPretFailed,           // FTP PRET 命令失败
  kCurleRtspCseqError,           // RTSP CSeq 数字不匹配
  kCurleRtspSessionError,        // RTSP 会话 ID 不匹配
  kCurleFtpBadFileList,          // 无法解析 FTP 文件列表
  kCurleChunkFailed,             // 块回调报告错误
  kCurleNoConnectionAvailable,   // 没有可用的连接，会话将排队等待
  kCurleSslPinnedpubkeynotmatch, // 指定的固定公钥不匹配
  kCurleSslInvalidcertstatus,    // 证书状态无效
  kCurleHttp2Stream,             // HTTP/2 协议帧层面的流错误
  kCurleRecursiveApiCall,        // 回调函数中调用了 API 函数
  kCurleAuthError,               // 认证问题
  kCurleHttp3,                   // HTTP/3 层面的问题
  kCurleQuicConnectError,        // QUIC 连接层面的问题
  kCurleProxy,                    // 代理服务器错误
  kCurleSslClientCert,            // SSL 客户端证书问题 

  // ed2k
  kEd2kTaskExisted = 2000,      // ed2k任务已存在
  kEd2kLinkInvalid,            // ed2k链接无效
  kWriteEd2kFileFailed,        // 写ed2k文件失败

  // magnet
  kMagnetTaskExisted = 3000,    // magnet任务已存在
  kMagnetLinkInvalid,          // magnet链接无效
  kAddMagnetTaskFailed,        // 添加magnet失败
  kMagnetParseFailed,          // magnet解析失败

  // torrent file
  kTorrentFileTaskExisted = 4000, // bt种子文件任务已存在
  kTorrentFileInvalid,           // bt种子文件无效
  kAddTorrentTaskFailed,           // 添加bt种子文件失败
  kTorrentFileParseFailed,          // bt种子解析失败

  // pause
  kPauseTaskFailed = 5000,       // 暂停任务失败

  // resume
  kResumeTaskFailed = 5100,// 恢复任务失败

  // delete
  kDeleteTaskFailed = 5200,// 删除任务失败
  kDeleteLocalFileFailed, // 删除本地文件失败

  // retry
  kRetryTaskFailed = 5300,// 重试任务失败

  // clean
  kCleanTaskFailed = 5400,// 清理失效任务失败

  // restore
  kRestoreTaskFailed = 5500,// 还原任务失败

  // db
  kDbFailed = 6000, // db失败

  // http_error
  kHttpStartErrorCode = 10000, // http起始错误码
  kHttpError400 = kHttpStartErrorCode + 400, // 请求无效
  kHttpError401 = kHttpStartErrorCode + 401, // 无法进行身份验证
  kHttpError403 = kHttpStartErrorCode + 403, // 没有访问权限
  kHttpError404 = kHttpStartErrorCode + 404, // 找不到页面
  kHttpError408 = kHttpStartErrorCode + 408, // 请求超时
  kHttpError500 = kHttpStartErrorCode + 500, // 服务器错误
  kHttpError501 = kHttpStartErrorCode + 501, // 不支持的操作
  kHttpError502 = kHttpStartErrorCode + 502, // 网关错误
  kHttpError503 = kHttpStartErrorCode + 503, // 服务不可用
  kHttpError504 = kHttpStartErrorCode + 504  // 网关超时
};

// 文件基本信息
struct FileBaseInfo {
    FileBaseInfo() {
        memset(file_name, 0, sizeof(file_name));
        file_size = 0;
        memset(file_type, 0, sizeof(file_type));
    }
    char file_name[kMaxPath];     // 文件名称
    unsigned long long file_size;   // 文件大小
    char file_type[32];             // 文件类型
};

// 下载统计信息
struct DownloadStatisticsInfo {
  DownloadStatisticsInfo() {
    download_rate = 0.0;
    upload_rate = 0.0;
    accumulate_download = 0;
    accumulate_upload = 0;
  }
  double download_rate;         // 下载速率，单位字节/s
  double upload_rate;           // 上传速率，单位字节/s
  unsigned long long accumulate_download;   // 累计下载，单位字节
  unsigned long long accumulate_upload;     // 累计上传，单位字节
};

// 下载参数
struct DownloadParam {
  DownloadParam() {
    memset(id, 0, sizeof(id));
    memset(url, 0, sizeof(url));
    memset(save_path, 0, sizeof(save_path));
    memset(file_name, 0, sizeof(file_name));
  }

  char id[kIdLength];                   // 任务id
  char url[kUrlLength];                // url链接
  char save_path[kMaxPath];      // 保存路径
  char file_name[kMaxPath];      // 文件名称
  DownloadType download_type;   // 下载类型
};

// 下载进度
struct DownloadProgressInfo {
  DownloadProgressInfo() {
    memset(file_name, 0, sizeof(file_name));
    total_progress = 0.0;
    download_speed = 0.0;
    upload_speed = 0.0;
    err_code = kNormal;
  }
  char file_name[kMaxPath];      // 文件名称
  double total_progress;          // 下载进度，0-100
  double download_speed;           // 下载速度，单位字节/s
  double upload_speed;             // 上传速度，单位字节/s
  DownloadErrorCode err_code;      // 错误码
};

// 解析结果
struct ParseResult {
    FileBaseInfo file_info;            // 文件信息
};

// 下载接口
class DownloadTaskInterface {
 public:
  virtual ~DownloadTaskInterface() = default;
  // 开始任务
  virtual DownloadErrorCode Start() = 0;
  // 暂停任务
  virtual bool Pause() = 0;
  // 恢复任务（可恢复暂停任务以及失败任务）
  virtual bool Resume(bool is_failed) = 0;
  // 删除任务
  virtual bool Cancel(bool is_delete_local_file) = 0;
  // 更新进度信息
  virtual bool GetProgressInfo(DownloadProgressInfo* info) = 0;
  // 根据任务是否完成来检验本地文件是否有效，未完成会有临时文件，完成后重命名了临时文件
  virtual bool Valid(bool is_finished) = 0;
  // 获取任务hash
  virtual const char* GetTaskHash() = 0;
  // 释放内存
  virtual void ReleaseBuf(const char* buf) = 0;
};

#endif // NAS_THIRD_PARTY_DOWNLOAD_DOWNLOAD_PUBLIC_DEFINE_H_
