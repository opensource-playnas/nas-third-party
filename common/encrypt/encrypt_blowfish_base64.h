/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2022-11-24 16:06:14
 */

#ifndef NAS_COMMON_ENCRYPT_ENCRYPT_BLOWFISH_BASE64_H_
#define NAS_COMMON_ENCRYPT_ENCRYPT_BLOWFISH_BASE64_H_

#include <string>

namespace nas {

bool Encrypt_Blowfish_Base64(const std::string& plain_text,
                             const std::string& key,
                             std::string& cipher_text);

bool Decrypt_Base64_Blowfish(const std::string& cipher_text,
                             const std::string& key,
                             std::string& plain_text);
}  // namespace nas

#endif  // NAS_COMMON_ENCRYPT_ENCRYPT_BLOWFISH_BASE64_H_