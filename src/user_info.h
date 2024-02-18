#ifndef __SEL_USERINFO__
  #define __SEL_USERINFO__

#include "constant_def.h"

extern const uint32_t uid_len;     // long 类型十六进制数的字符串长度
extern const uint32_t name_limit;  // 用户名上限

typedef struct user_info_t {
  char *name; // 用户名即家目录名
  long uid;

  void (*destructor)(struct user_info_t* const);
  struct user_info_t* (*set_uid)(struct user_info_t* const);
} usr_info;

usr_info* create_usr(usr_info* const this);
void release_usr(usr_info* const this);
usr_info* generate_uid(usr_info* const this);

// 工厂函数，创建一个新的 usr 对象，失败返回 NULL 指针
usr_info* make_usr_info(usr_info* const this);

#endif
