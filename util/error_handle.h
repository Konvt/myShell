#ifndef __SEL_ERROR_HANDLE__
  #define __SEL_ERROR_HANDLE__

typedef enum error_code_t {
  success, failed,
  constructor_error, init_error,
  received_EOF
} error_code;

/// @brief 将错误原因抛出到 `stderr` 中
/// @param where 错误位置
/// @param what 错误原因，不要带换行符
void throw_error(const char* where, const char* what);

#endif
