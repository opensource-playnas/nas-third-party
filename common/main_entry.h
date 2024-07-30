/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: panming@ludashi.com
 * @Date: 2023-07-28 16:30:16
 */

#ifndef NAS_COMMON_MAIN_ENTRY_H_
#define NAS_COMMON_MAIN_ENTRY_H_

// 替换main入口，每个模块的入口换成这个，进入此函数之前已经初始化了commondline了
int MainEntry();

#endif //NAS_COMMON_MAIN_ENTRY_H_