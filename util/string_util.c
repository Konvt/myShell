#include <string.h> // strstr
#include <ctype.h>  // isspace

#include "string_util.h"

char* trim_str(char* str, const size_t terminus)
{
  if (str == NULL || terminus == 0)
    return NULL; // an error offset

  char* cur = NULL; // 去尾
  for (cur = str + terminus - 1; cur != str; --cur) {
    if (isspace(*cur)) // 仅跳转到 '\0' 字符处，避免越界
      *cur = NIL_CHAR;
    else if (*cur != NIL_CHAR)
      break; // 规避 '\0' 字符
  } // 掐头
  for (cur = str; cur != str + terminus - 1; ++cur) {
    if (isspace(*cur)) // str + terminus - 1 是 '\0'，不用处理
      *cur = NIL_CHAR;
    else break;
  }

  return cur;
}

char* match_str(const char* mainstr, const char* substr, const size_t pos)
{
  return strstr(mainstr + pos, substr);
}

void format_str(char* str, const char* target)
{
  const char sep = '/';
  const char* terminus = str;
  while (*terminus++ != NIL_CHAR);
  const size_t sub_len = strlen(target);
  size_t pos = 0;
  do {
    char *substr = match_str(str, target, pos);
    pos += sub_len;

    if (substr == NULL || substr == str) break;
    if (substr != str && *(substr - 1) != sep)
      continue; // 不满足 `/%s`
    // 检查是否满足 `/%s/` 或 `/%s`，后一种情况仅限 substr 在 str 末尾
    if (*(substr + 1) != NIL_CHAR && *(substr + sub_len) != sep)
      if (*(substr + sub_len) != NIL_CHAR)
        continue;

    str[0] = '~';
    strcpy(str + 1, substr + sub_len);

    char *end = str;
    while (*end++ != NIL_CHAR);
    memset(end, NIL_CHAR, terminus - end);
    break;
  } while (pos < terminus - str);
}
