/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-02-01 15:00:40
 */

#ifndef NAS_GATEWAY_REQUEST_HANDLER_ID_HPP_
#define NAS_GATEWAY_REQUEST_HANDLER_ID_HPP_

#include <iostream>

namespace nas {

// 定义服务访问id,每个服务间id 间隔100
enum class HandlerRequestId : uint32_t {
  // 网关提供的服务定义在1 - 99之间
  kServicePing = 1,
  kServiceStatus,
  kServiceStop,

  // 鉴权服务访问保留范围100 - 199
  // 请求私有云设备id
  kDeviceId = 100,
  // 对请求进行鉴权
  kVerifyRequest,
  // 请求绑定私有云设备
  kDeviceBind,
  // 换绑私有云
  kExchangeBind,
  // 重置设备绑定状态
  kRetsetDeviveBind,
  // 使用中心服务器返回的code进行登录,换取真正的access_token
  kCodeLogin,
  // 使用access token登录
  kAccessTokenLogin,
  // 从系统中登出
  kLogout,
  // 获取当前系统中的有效token
  kGetAllTokens,
  // 添加用户
  kAddNasUserInfo,
  // 获取Nas设备的基本信息
  kGetNasBaseInfo,
  // 获取用户的悬空付费套餐
  kGetUserPayPlan,
  // 获取当前设备使用的套餐
  kGetCurPayPlan,
  // 选择一个套餐给设备用
  kSelectPayPlan,
  // 获取当前用户的基本信息
  kGetCurUserInfo,

  // 消息中心服务器建立的双向通道
  kMessageCenterId = 200,
  // 发布订阅
  kMessagePubSub,
  // 请求历史消息
  kMessageHistory,
  // 删除消息
  kMessageDelId,
  kMessageDelType,
  kMessageAdd,

  // 文件服务
  kFileService = 300,
  // 获取磁盘大小
  kDiskSize,
  // 创建文件
  kFileCreate,
  // 遍历指定目录下的文件
  kFileList,
  // 获取指定文件夹属性
  kFolderDetails,
  // 删除文件
  kFileDelete,
  // 重命名文件
  kFileRename,
  // 移动文件
  kFileMove,
  // 复制文件
  kFileCopy,
  // 文件搜索
  kFileSearch,
  // 文件操作取消
  kFileOperatorCacel,
  // 增加收藏夹
  kFavoriteAdd,
  // 遍历收藏就列表
  kFavoriteList,
  // 收藏夹改变顺序
  kFavoriteMove,
  // 取消收藏
  kFavoriteCancel,
  // 创建分享链接
  kCreateShareLink,
  // 删除分享链接
  kDeleteShareLink,
  // 删除无效分享链接
  kDeleteInvalidShareLink,
  // 列出分享内容
  kListShareLink,
  // 获取分享链接属性
  kGetShareLink,
  // 重置分享有效期
  kResetShareLink,
  // list 文件操作任务
  kListTaskRecords,
  // 删除 从数据库中删除记录的操作任务
  kDeleteTaskRecords,
  // 判断路径列表是否都存在
  kPathListExists,
  // 获取路径id
  kGetIDByPath,
  // 获取路径（通过路径id）
  kGetPathByID,
  // 文件打包
  kFilePack,
  // 文件解压
  kFileUnPack,
  // 获取压缩文件的信息
  kArciveInfo,
  // 获取指定类型文件
  kGetSpecifiedTypeFiles,

  // 媒体服务
  kMeidaService = 400,
  // 视频信息
  kVideoInfo,
  // 直接播放视频文件
  kVideoPlayByFile,
  // 创建视频播放任务
  kVideoPlay,
  // 播放视频的TS文件
  kVideoPlayTs,
  // 根据视频id 获取字幕文件
  kVideoSubtitle,
  // 转码信息
  kVideoPlayTransCode,
  // 取消播放任务
  kVideoPlayCancel,
  // 查询播放任务数量
  kVideoQueryPlayTaskInfo,

  // 图片服务
  kPictureService = 500,
  // 获取缩略图路径
  kPictureThumbnailPath,
  kPictureWallpaperCollectionList,
  kPictureWallpaperAddCollection,
  kPictureWallpaperDelCollection,
  kPictureWallpaperFileList,
  kPictureWallpaperAddFile,
  kPictureWallpaperDelFile,
  kPictureWallpaperSetSetting,
  kPictureWallpaperGetSetting,
  kPictureFileTranscode,
  kPictureGalleryGetMetaInfo,

  // 远程下载服务
  kDownloadService = 600,
  // 创建http/https、ftp/ftps、ed2k的链接下载任务
  kCreateLinkDownloadTask,
  // 创建magnet链接和BT文件下载任务
  kCreateTorrentDownloadTask,
  // 上传种子文件
  kUploadTorrentFile,
  // 暂停任务
  kPause,
  // 暂停所有正在下载任务
  kPauseAll,
  // 恢复任务
  kResume,
  // 恢复所有暂停任务
  kResumeAll,
  // 删除任务
  kDelete,
  // 删除分类
  kDeleteCategory,
  // 重试失败任务
  kRetry,
  // 重试所有失败任务
  kRetryAll,
  // 清除无效任务
  kClean,
  // 清除分类无效任务
  kCleanCategory,
  // 还原回收站任务
  kRestore,
  // 还原所有回收站任务
  kRestoreAll,
  // 获取所有任务信息
  kGetTaskInfo,
  // 获取链接信息
  kGetLinkInfo,
  // 获取magnet和种子文件信息
  kGetTorrentInfo,
  // 获取种子文件上传路径
  kGetTorrentUploadPath,

  // 检查更新
  kQueryUpdateInfo = 700,
  // 下载升级包
  kDownloadUpdatePackage,
  // 取消下载
  kCancelDownloadUpdatePackage,
  // 安装升级包
  kInstallUpdatePackage,
  // 查询安装结果
  kCheckInstallResult,

  // 配置服务
  kStorageService = 800,
  // 创建或修改字段值
  kPutStorage,
  // 获取指定namespace配置
  kGetStorage,
  // 列出多个namespace配置
  kListStorage,
  // 删除配置
  kDeleteStorage,
  // 重置配置
  kResetStorage,

  // 系统信息服务
  kSystemInfoService = 900,
  // 获取硬件信息
  kHadrewareInfo,
  // 获取网络信息
  kNetInfo,
  // 获取基本信息
  kGetBaseStaticInfo,
  // 设置基本信息
  kSetBaseStaticInfo,
  // 获取时间信息
  kGetTimeStaticInfo,
  // 获取缓存大小
  kGetCacheInfo,
  // 清除缓存
  kCleanCache,
  // 重启本机操作系统
  kRestartOS,
  // 关机本机操作系统
  kShutdownOs,
  // 反馈接口
  kFeedback,
  // Dlna 服务
  kDlnaService = 1000,
  // 获取可投屏设备列表
  kListDlnaDevice,
  // 创建投屏任务
  kCreateDlnaTask,
  // 取消投屏任务
  kCancelDlnaTask,
  // 改变投屏任务播放源
  kChangeDlnaTaskPlayUrl,
  // 播放投屏
  kPlayDlnaTask,
  // 暂停投屏
  kPauseDlnaTask,
  // 跳转
  kSeekDlnaTask,
  // 设置音量
  kSetDlnaTaskVolume,

  // grpc基础服务接口
  // 重置接口,重置grpc内部的数据
  kResetGrpcService = 1100,
  // 停止grpc服务
  kStopGrpcService,

  // ServiceManager
  // 注册服务
  kRegisterService = 1200,
  // 修改服务注册信息
  kModifyRegister,
  //查询指定服务信息
  kQueryServiceInfo,
  //查询所有服务的状态
  kQueryServiceStatus,
  // 初始化php
  kLaunchInitPhp,

  // 健康检查
  kHealthCheck = 1300
};  // enum class HandlerRequestId

// 通知中心的操作Id定义
enum class MessageCenterOperatorId : uint32_t {
  // 连接到消息中心的请求
  kConnect = 0,
  // 重连
  kReconnect,
  // 需要从消息中心断开,断开后不能再请求数据
  kDisconnect,
  // 订阅登录用户的消息
  kSubscribe,
};  // enum class MessageCenterOperator

}  // namespace nas

#endif  // NAS_GATEWAY_REQUEST_HANDLER_ID_HPP_