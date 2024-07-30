

#include <string>

#include "base/check.h"
#include "base/strings/string_util.h"

#include "base64/base64.h"

#include "blowfish/blowfish.h"
#include "encrypt_blowfish_base64.h"

namespace nas {

bool Encrypt_Blowfish_Base64(const std::string& plain_text,
                             const std::string& key,
                             std::string& cipher_text) {
  bool ret = false;
  BYTE* output = nullptr;
  do {
    DCHECK(!plain_text.empty());
    if (plain_text.empty())
      break;

    CBlowFish codec;
    codec.Initialize((const BYTE*)key.c_str(), key.length());

    uint32_t output_len = codec.GetOutputLength(plain_text.length());
    if (output_len <= 0)
      break;

    output = new BYTE[output_len];
    if (output == NULL)
      break;

    memset(output, 0, output_len);
    if (!codec.Encode((BYTE*)plain_text.c_str(), output, plain_text.length())) {
      break;
    }

    if (!XTL::CXtlBase64::encode(cipher_text, (const char*)output,
                                 output_len)) {
      break;
    }

    ret = true;
  } while (0);

  if (output) {
    delete[] output;
  }

  return ret;
}

bool Decrypt_Base64_Blowfish(const std::string& cipher_text,
                             const std::string& key,
                             std::string& plain_text) {
  bool ret = false;

  BYTE* base64_output = NULL;
  BYTE* output = NULL;

  do {
    DCHECK(!cipher_text.empty());
    if (cipher_text.empty())
      break;

    size_t base64_output_size = modp_b64_decode_len(cipher_text.length());
    if (base64_output_size <= 0)
      break;

    base64_output = new BYTE[base64_output_size];
    if (base64_output == NULL)
      break;

    memset(base64_output, 0, base64_output_size);
    if (!XTL::CXtlBase64::decode((char*)base64_output, base64_output_size,
                                 cipher_text))
      break;

    CBlowFish codec;
    codec.Initialize((const BYTE*)key.c_str(), key.length());

    uint32_t output_len = codec.GetOutputLength(base64_output_size);
    if (output_len <= 0)
      break;

    output = new BYTE[output_len + 1];
    if (output == NULL)
      break;

    memset(output, 0, output_len + 1);
    if (!codec.Decode((BYTE*)base64_output, output, base64_output_size)) {
      break;
    }

    // 去掉多余的\0
    while((output_len > 0) && (output[--output_len] == '\0'));
    output_len++;

    plain_text = std::string((char*)output, output_len);

    ret = true;

  } while (0);

  if (base64_output) {
    delete[] base64_output;
  }

  if (output) {
    delete[] output;
  }

  return ret;
}

}  // namespace nas