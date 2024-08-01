/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:52:38
 */

#ifndef NAS_THIRD_PARTY_DOWNLOAD_NETCURL_NETCURL_SRC_CURL_EXPORT_DEFINE_H_
#define NAS_THIRD_PARTY_DOWNLOAD_NETCURL_NETCURL_SRC_CURL_EXPORT_DEFINE_H_

#ifdef _WINDOWS
#include <windows.h>
#define EXPORT __declspec(dllexport)
#define DECL_NAME __cdecl
#else
#define EXPORT __attribute__((visibility("default")))
#define DECL_NAME __attribute__((__cdecl__))
#endif

#ifdef _WINDOWS
#ifdef _WIN64
#ifdef _DEBUG
#pragma comment(lib, "libssl_T64d.lib")
#pragma comment(lib, "libcrypto_T64d.lib")
#pragma comment(lib, "libcurl_T64d.lib")
#pragma comment(lib, "zlib_mtd_64.lib")
#else
#pragma comment(lib, "libssl_T64.lib")
#pragma comment(lib, "libcrypto_T64.lib")
#pragma comment(lib, "libcurl_T64.lib")
#pragma comment(lib, "zlib_mt_64.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "libssl_d.lib")
#pragma comment(lib, "libcrypto_d.lib")
#pragma comment(lib, "libcurld.lib")
#pragma comment(lib, "libssh2_d.lib")
#else
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "libssh2.lib")
//#pragma comment(lib, "libcares.lib")
#endif
#endif

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Userenv.lib")

#endif

#endif
