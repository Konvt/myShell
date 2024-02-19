#ifndef __SEL_STR_UTIL__
  #define __SEL_STR_UTIL__

#include "constant_def.h"

/// @brief 裁切字符串指定范围内的首尾空白字符
/// @param str 待处理字符串
/// @param terminus 和参数 `str` 构成一个左闭右开的区间 `[str, str + terminus)`
/// @return 指向第一个非空字符的指针，参数非法时返回 NULL
char* trim_str(char* const str, const size_t terminus);

/// @brief 从 `pos` 开始匹配 `substr`
/// @param mainstr 主串
/// @param substr 子串
/// @param pos 起始偏移量
/// @return `substr` 在 `mainstr` 中的起始位置，不存在则返回 `NULL`
char* match_str(const char* mainstr, const char* substr, const size_t pos);

/// @brief 将 `str` 中的 `target` 替换为 `~`，并前移字符串，
/// @brief 其实就是替换目录字符串中的家目录名称
/// @param str 待处理的字符串
/// @param target 目标字符串，默认该字符串满足格式 `/%s/` 或 `/%s`
void format_str(char* str, const char* target);

#endif
