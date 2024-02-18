#ifndef __SEL_RUNTIME__
  #define __SEL_RUNTIME__

#include <stdio.h> // FILE

#include "constant_def.h"
#include "user_info.h"

// 定义表达式长度值的底层数据类型
typedef uint32_t ExprLenT;
extern const ExprLenT read_limit;

typedef enum expression_t {
  /* the first member of each line is used as separator and has no real meaning */
  nil,
  external, externCopy, externMkdir, externTouch, externLook, externLs,
  buildin, buildinExit, buildinEcho, buildinHelp, buildinPwd, buildinCd, buildinCls,
  gotoMoon, fiveFives,
} expr_type;

typedef struct shell_runtime_t {
// private:
  char *_origianl_expr;  // 原始命令串
  char *_processed_expr; // 经过裁切处理后的命令
  ExprLenT _expr_len;
// public:
  usr_info* active_usr;   // 当前使用该 shell 的用户
  char* cwd; // 当前工作目录

  int argc;
  char **args;
  expr_type prev_exprT;

  void (*destructor)(struct shell_runtime_t* const);
  struct shell_runtime_t* (*scanner)(struct shell_runtime_t* const, FILE *, const char*);
  void (*interpreter)(struct shell_runtime_t* const);
} shell_rt;

shell_rt* create_shell(shell_rt* const this, usr_info* logged_usr);
void release_shell(shell_rt* const this);
// 获取表达式并做词法分析
shell_rt* get_expr(shell_rt* const this, FILE* infile, const char* prompt);
// 解释执行处理后的表达式
void interprete_epxr(shell_rt* const this);

/// @brief 从指定的文件标识符中读取命令表达式
/// @param infile 指定的文件标识符，一般是 stdin
/// @param prompt 需要输出的命令提示符
shell_rt* read_expr(shell_rt* const this, FILE* infile, const char* prompt);

/// @brief 对原始表达式进行词法分析
/// @param delims 词法分析时用于拆分 token 的字符集合
shell_rt* analyse_expr(shell_rt* const this);

/* pipe and redirect will be processed separately */
int execute_pipe(char** args, int argc, int pipe_flag);
int execute_redirect(char** args, int argc, int redirect_flag);
expr_type categorize_epxr(char** args, int argc);
int buildin_expr(char** args, int argc, expr_type type);
int external_expr(char** args, int argc, expr_type type);

#endif
