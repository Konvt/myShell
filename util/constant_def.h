#ifndef __SEL_DEFINITION__
  #define __SEL_DEFINITION__

#include <stddef.h>
#include <stdint.h>

#define NIL_CHAR '\0'

// 宏函数，用于释放并置空一个指针
#define release_ptr(ptr) do { free(ptr); ptr = NULL; } while (0)

// 一次性释放并置空多个指针，使用时传入指针变量的地址
#define release_multiptrs(...) do { \
  void **ptrs[] = { __VA_ARGS__ }; \
  for (size_t i = 0; i < sizeof(ptrs) / sizeof(ptrs[0]); ++i) \
    release_ptr(*ptrs[i]); \
} while (0)

#endif
