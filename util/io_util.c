#include <string.h> // memset
#include <stdlib.h> // exit

#include "io_util.h"
#include "error_handle.h"
#include "string_util.h"

void input_str(FILE* infile, char* dest, int char_limit, const char* prompt)
{
  if (dest == NULL || char_limit == 0)
    return;

  memset(dest, NIL_CHAR, char_limit * sizeof(char));
  char *trimmed = NULL; fflush(infile);
  do { // 持续获取输入，直到获取到非空输入
    if (prompt != NULL && dest[0] == NIL_CHAR)
      fputs(prompt, stdout); // 输出提示符
    if (fgets(dest, char_limit, infile) == NULL) {
      putchar('\n'); // 接收到 EOF，终止进程
      exit(received_EOF);
    }
    trimmed = trim_str(dest, strlen(dest) + 1);
  } while (trimmed[0] == NIL_CHAR);

  /* 如果输入的字符串前半部分是一大堆空白字符
   * 会导致移动非空字符串时，在主串 dest 后面留下一些非空字符
   * 这里负责把遗留的非空字符清空 */
  if (dest != trimmed) {
    strcpy(dest, trimmed);
    char *end = dest;
    while (*end++ != NIL_CHAR);
    memset(end, NIL_CHAR, (dest + char_limit) - end);
  }
}
