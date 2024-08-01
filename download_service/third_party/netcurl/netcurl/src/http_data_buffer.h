// copyright 2022 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2022/11/03 10:30

#ifndef NAS_INSTALLER_UNINSTALLER_SRC_NETWORK_HTTP_DATA_BUFFER_H_
#define NAS_INSTALLER_UNINSTALLER_SRC_NETWORK_HTTP_DATA_BUFFER_H_

namespace nas {

class HttpDataBuffer {
 public:
  HttpDataBuffer() : block_size(1024 * 1024ul) {
    buffer_ptr = (char*)malloc(block_size);
    buffer_size = block_size;
    buffer_length = 0;
  }

  ~HttpDataBuffer() {
    if (buffer_ptr) {
      free(buffer_ptr);
      buffer_ptr = NULL;
    }
    buffer_length = 0;
    buffer_size = 0;
  }

  bool AppendBuffer(char* data, unsigned long len) {
    // add '\0' to data's end.
    if (buffer_length + len > (buffer_size - 1)) {
      buffer_size += len + block_size;
      buffer_ptr = (char*)realloc(buffer_ptr, buffer_size);
    }
    if (buffer_ptr) {
      memcpy(&(buffer_ptr[buffer_length]), data, len);
      buffer_ptr[buffer_length + len] = '\0';
      buffer_length += len;
    }

    return NULL != buffer_ptr;
  }

  char* GetBuffer() { return buffer_ptr; }

  unsigned long GetLength() { return buffer_length; }

 private:
  //默认增加一个固定大小的缓冲区,
  const unsigned long block_size;

  char* buffer_ptr;
  unsigned long buffer_size;
  unsigned long buffer_length;
};
}  // namespace nas

#endif  // NAS_INSTALLER_UNINSTALLER_SRC_NETWORK_HTTP_DATA_BUFFER_H_
