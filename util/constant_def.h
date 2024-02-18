#ifndef __SEL_DEFINITION__
  #define __SEL_DEFINITION__

#include <stddef.h>
#include <stdint.h>

#define NIL_CHAR '\0'

// 宏函数，用于释放并置空一个指针
#define release_ptr(ptr) do { free(ptr); ptr = NULL; } while (0)

#endif
