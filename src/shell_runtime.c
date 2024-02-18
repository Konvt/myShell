#include <string.h> // strlen, strcpy, memset
#include <stdlib.h> // calloc, free

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "io_util.h"
#include "error_handle.h"
#include "shell_runtime.h"
#include "functional_func.h"

const ExprLenT read_limit = 1024;

shell_rt* create_shell(shell_rt* const this, usr_info* logged_usr)
{
  this->_origianl_expr = (char*)calloc(sizeof(char), read_limit + 1);
  if (this->_origianl_expr == NULL)
    return NULL;
  this->_processed_expr = (char*)calloc(sizeof(char), read_limit + 1);
  if (this->_processed_expr == NULL) {
    release_ptr(this->_origianl_expr);
    return NULL;
  }
  this->_expr_len = 0;

  this->active_usr = logged_usr;
  this->cwd = get_cwd(this->active_usr->name);
  if (this->cwd == NULL) {
    release_ptr(this->_origianl_expr);
    release_ptr(this->_origianl_expr);
    return NULL;
  }
  this->args = NULL; this->argc = 0;
  this->prev_exprT = nil;

  this->destructor = &release_shell;
  this->scanner = &get_expr;
  this->interpreter = &interprete_epxr;

  return this;
}

void release_shell(shell_rt* const this)
{
  release_ptr(this->_origianl_expr);
  release_ptr(this->_processed_expr);
  release_ptr(this->args);

  this->_expr_len = 0;
  this->argc = 0;
  this->active_usr = NULL;
  this-> prev_exprT = nil;
}

shell_rt* get_expr(shell_rt* const this, FILE* infile, const char* prompt)
{
  if (read_expr(this, infile, prompt) == NULL) {
    throw_error("scanner", "read expression error");
    return NULL;
  }
  if (analyse_expr(this) == NULL) {
    throw_error("scanner", "lexical analysis error");
    return NULL;
  }
  return this;
}

void interprete_epxr(shell_rt* const this)
{
  this->prev_exprT = nil;
  if (this->args[0] == NULL || this->args[0][0] == NIL_CHAR)
    return; // if command is a line of space
  int execute_flag = failed, pipe_flag = -1, redirect_flag = -1;

  for (int i = 0; i < this->argc; i++) {
    if (strcmp(this->args[i], "|") == 0)
      pipe_flag = i;
    else if (strcmp(this->args[i], ">") == 0)
      redirect_flag = i;
  }
  /* they are mutually exclusive */
  if (pipe_flag != -1 && redirect_flag != -1) {
    throw_error("shell", "unsupported command");
    return;
  }

  if (pipe_flag != -1) {
    execute_flag = execute_pipe(this->args, this->argc, pipe_flag);
  } else if (redirect_flag != -1) {
    execute_flag = execute_redirect(this->args, this->argc, redirect_flag);
  } else {
    this->prev_exprT = categorize_epxr(this->args, this->argc);
    if (this->prev_exprT > buildin)
      execute_flag = buildin_expr(this->args, this->argc, this->prev_exprT);
    else if (this->prev_exprT < buildin)
      execute_flag = external_expr(this->args, this->argc, this->prev_exprT);
  }

  if (this->prev_exprT == buildinCd) {
    release_ptr(this->cwd);
    this->cwd = get_cwd(this->active_usr->name);
    if (this->cwd == NULL) {
      throw_error("shell", "update cwd failure");
      exit(failed); // 不退出也只会发生段错误，即访问空指针
      // 到这里程序流已经太深了，没法执行资源清理然后逐级退出
      // 所以只能执行 exit 强行终止进程
    }
  }

  // 除了 error_code::external 以外的指令处理都有自己的报错信息
  // 所以这里要特殊处理
  if (execute_flag == failed && this->prev_exprT == external)
    throw_error(this->_origianl_expr, "command error or not found");
}

shell_rt* read_expr(shell_rt* const this, FILE* infile, const char* prompt)
{
  input_str(stdin, this->_origianl_expr, read_limit, prompt);
  this->_expr_len = strlen(this->_origianl_expr);

  memset(this->_processed_expr, NIL_CHAR, read_limit + 1);
  strcpy(this->_processed_expr, this->_origianl_expr);

  return this;
}

shell_rt* analyse_expr(shell_rt* const this)
{
  this->argc = 0;
  /* cut tokens by space */
  for (char *str = strtok(this->_processed_expr, " "); str != NULL;
       str = strtok(NULL, " "), this->argc++);

  release_ptr(this->args);
  this->args = (char**)calloc(this->argc + 1, sizeof(char*));
  if (this->args == NULL) 
    return NULL;

  /* find the token and assign their starting address to args */
  for (uint32_t i = 1, backtrack = 1, num_args_saved = 0;
       i <= (this->_expr_len) && num_args_saved < this->argc; ++i, ++backtrack) {
    if (this->_processed_expr[i] != NIL_CHAR && this->_processed_expr[i - 1] == NIL_CHAR)
      backtrack = 0; // backtrack is used to backtrack to the head of each token
    else if (this->_processed_expr[i] == NIL_CHAR && this->_processed_expr[i - 1] != NIL_CHAR) {
      this->args[num_args_saved++] = &this->_processed_expr[i - backtrack];
      backtrack = 0;
    }
  }

  return this;
}

int execute_pipe(char** args, int argc, int pipe_flag)
{
  if (argc < 3 || pipe_flag == 0 || pipe_flag == argc - 1)
    return failed;
  /* 将 '|' 左侧的 stdout 重定向到右侧的 stdin */
  int stream_pipe[2], status_pipe[2];
  if (pipe(stream_pipe) == -1 || pipe(status_pipe) == -1) {
    throw_error("pipe", "create pipe error");
    return failed;
  }
  fcntl(status_pipe[0], F_SETFL, fcntl(status_pipe[0], F_GETFL) | O_NONBLOCK);
  /* 避免无输出的指令阻塞了进程 */
  fcntl(stream_pipe[0], F_SETFL, fcntl(stream_pipe[0], F_GETFL) | O_NONBLOCK);

  expr_type type[2] = {nil, nil};
  int execute_flag = success;
  type[0] = categorize_epxr(args, argc);
  type[1] = categorize_epxr(args + pipe_flag + 1, argc - pipe_flag - 1);

  pid_t pid[2];
  for (int num_expr = 0; num_expr < 2; num_expr++) {
    /* 由于所有指令都会 fork 后由子进程执行，故在管道指令中使用 exit 是没用的 */
    pid[num_expr] = fork();
    if (pid[num_expr] == 0) {
      close(status_pipe[0]);
      /* in front of '|' */
      if (num_expr == 0) {
        close(stream_pipe[0]);
        /* 1 is stdout */
        dup2(stream_pipe[1], 1);
        /* reorganizes args and argc that the current process should process */
        args[pipe_flag] = NULL;
        argc = pipe_flag;
        /* at the back of '|' */
      } else if (num_expr == 1) {
        close(stream_pipe[1]);
        /* 0 is stdin */
        dup2(stream_pipe[0], 0);
        args = args + pipe_flag + 1;
        argc = argc - pipe_flag - 1;
      }
      if (type[num_expr] > buildin)
        execute_flag = buildin_expr(args, argc, type[num_expr]);
      else {
        execute_flag = external_expr(args, argc, type[num_expr]);
        write(status_pipe[1], &execute_flag, sizeof(int));
      }
      close(stream_pipe[0]);
      close(stream_pipe[1]);
      exit(success);
    }
    /* there will be nothing to be read if we don't wait for subprocess end */
    waitpid(pid[num_expr], NULL, 0);
  }
  read(status_pipe[0], &execute_flag, sizeof(int));

  close(stream_pipe[0]); close(stream_pipe[1]);
  close(status_pipe[0]); close(status_pipe[1]);
  return execute_flag;
}

int execute_redirect(char** args, int argc, int redirect_flag)
{
  if (argc - redirect_flag - 1 > 1) {
    throw_error("redirect", "too many files");
    return failed;
  }
  if (access(args[redirect_flag + 1], F_OK) == -1) {
    if (touch_file(args + redirect_flag + 1, 1)) {
      throw_error("redirect", "create target file failed");
      return failed;
    }
  } else if (access(args[redirect_flag + 1], W_OK) == -1) {
    throw_error("redirect", "target file cannot be written");
    return failed;
  }

  int fd[2], execute_flag = success;
  if (pipe(fd) == -1)
    return failed;
  fcntl(fd[0], F_SETFL, fcntl(fd[0], F_GETFL) | O_NONBLOCK);

  expr_type type = categorize_epxr(args, argc);
  int output = open(args[redirect_flag + 1], O_WRONLY);
  if (fork() == 0) {
    /* redirect the stdout to the file 'output' */
    dup2(output, 1);
    close(fd[0]);
    args[redirect_flag] = NULL;
    argc = argc - redirect_flag - 1;
    if (type > buildin)
      execute_flag = buildin_expr(args, argc, type);
    else {
      execute_flag = external_expr(args, argc, type);
      write(fd[1], &execute_flag, sizeof(int));
    }
    close(fd[1]);
    exit(success);
  }
  wait(NULL);
  read(fd[0], &execute_flag, sizeof(int));

  close(output);
  close(fd[0]);
  close(fd[1]);
  return execute_flag;
}

int buildin_expr(char** args, int argc, expr_type type)
{ /* 用 const 约束 args 是无意义的，这是 C 的语言缺陷 */
  switch (type) {
  case buildinExit:
    exit(EXIT_SUCCESS);
  case buildinHelp:
    get_help();
    return success;
  case buildinEcho:
    return echo(args, argc);
  case buildinPwd:
    char *temp = get_cwd(NULL);
    puts(temp); release_ptr(temp);
    return success;
  case buildinCd:
    if (argc > 2) {
      throw_error("cd", "too many arguments");
      return failed;
    }
    return change_wd(args[1]);
  case buildinCls:
    return clean_up(args, argc);
  case gotoMoon:
    return goto_moon();
  case fiveFives:
    return five_fives();
  default:
    break;
  }

  return failed;
}

int external_expr(char** args, int argc, expr_type type)
{
  switch (type) {
  case externCopy:
    return copy_file(args, argc);
  case externMkdir:
    return make_dir(args, argc);
  case externTouch:
    return touch_file(args, argc);
  case externLook:
    return look_file(args, argc);
  case externLs:
    return list_dir(args, argc);
  case external:
  default:
    return execute(args, argc);
  }

  return failed;
}

expr_type categorize_epxr(char** args, int argc)
{
  switch (args[0][0]) {
  case 'e': /* exit */
    if (strcmp(args[0], "exit") == 0)
      return buildinExit;
    else if (strcmp(args[0], "echo") == 0)
      return buildinEcho;
    break;
  case 'h': /* help */
    if (strcmp(args[0], "help") == 0)
      return buildinHelp;
    break;
  case 'p': /* pwd */
    if (strcmp(args[0], "pwd") == 0)
      return buildinPwd;
    break;
  case 'c': /* cd */
    if (strcmp(args[0], "cd") == 0)
      return buildinCd;
    else if (strcmp(args[0], "cp") == 0)
      return externCopy;
    else if (strcmp(args[0], "cls") == 0)
      return buildinCls;
    break;
  case 'm': /* mkdir */
    if (strcmp(args[0], "mkdir") == 0)
      return externMkdir;
    break;
  case 't': /* touch */
    if (strcmp(args[0], "touch") == 0)
      return externTouch;
    break;
  case 'd': /* dir */
  case 'l': /* ls */
    if (strcmp(args[0], "ls") == 0 || strcmp(args[0], "dir") == 0)
      return externLs;
    else if (strcmp(args[0], "look") == 0)
      return externLook;
    break;
  case 'g':
    if (argc == 2 && strcmp(args[0], "goto") == 0
        && strcmp(args[1], "moon") == 0)
      return gotoMoon;
    break;
  case '5':
    if (strcmp(args[0], "55555") == 0)
      return fiveFives;
  default:
    break;
  }

  return external;
}
