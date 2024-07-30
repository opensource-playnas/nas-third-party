// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/01/18 14:48
#include "utils.h"

#include "base/hash/md5.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "url/gurl.h"
namespace utils {

double Round(double number, unsigned int bits) {
  double integerpart = floor(number);
  number -= integerpart;
  for (unsigned int i = 0; i < bits; ++i) {
    number *= 10;
  }
  number = floor(number + 0.5);
  for (unsigned int i = 0; i < bits; ++i) {
    number /= 10;
  }
  return integerpart + number;
}

std::string GetFileExtensionFromUrl(const std::string& url_string) {
  GURL url(url_string);
  if (!url.is_valid()) {
    return "";
  }

  base::FilePath file_path =
      base::FilePath::FromUTF8Unsafe(url.ExtractFileName());
  base::FilePath::StringType extension = file_path.Extension();

  // Remove the leading dot from the extension, if any.
  if (!extension.empty() &&
      extension[0] == base::FilePath::kExtensionSeparator) {
    extension.erase(0, 1);
  }
#if BUILDFLAG(IS_WIN)
  return base::WideToUTF8(extension);
#else
  return extension;
#endif
}

}  // namespace utils
