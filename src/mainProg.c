#include <string.h> // strlen
#include <stdlib.h> // exit, calloc, free
#include <signal.h> // signal

#include "shell_runtime.h"
#include "error_handle.h"
#include "font_style.h"
#include "welcome.h"

int main()
{
  // 屏蔽 ctrl+c 中断
  signal(SIGINT, SIG_IGN);
  // 程序关闭只能使用 ctrl+d 输入 EOF

  usr_info user;
  shell_rt shell;
  if (make_usr_info(&user) == NULL) {
    throw_error("login", "create user failure");
    exit(constructorError);
  }
  if (create_shell(&shell, &user) == NULL) {
    throw_error("login", "create shell failure");
    exit(constructorError);
  }
  const char *prompt_fmt = STYLIZE("%s@%x", FNT_GREEN FNT_BOLD_TEXT) ":" STYLIZE("%s", FNT_BLUE FNT_BOLD_TEXT) "$ ";
  const size_t basic_len = name_limit + uid_len + strlen(prompt_fmt);

  size_t cwd_len = strlen(shell.cwd);
  size_t prompt_len = basic_len + cwd_len;

  char *prompt = (char*)calloc(prompt_len + 1, sizeof(char));
  if (prompt == NULL) {
    shell.destructor(&shell);
    user.destructor(&user);
    throw_error("prompt", "create prompt string failure");
    exit(initError);
  }
  sprintf(prompt, prompt_fmt, shell.active_usr->name, shell.active_usr->uid, shell.cwd); // splicing prompt

  welcome();
  while (1) {
    shell.scanner(&shell, stdin, prompt)->interpreter(&shell);;

    if (shell.prev_exprT == buildinCd) {
      cwd_len = strlen(shell.cwd);
      /* 保证 prompt 始终能够容纳下 cwd */
      if (prompt_len < basic_len + cwd_len) {
        /* 容纳不下时触发一次手动扩容 */
        prompt_len = basic_len + (cwd_len * 2);
        release_ptr(prompt); // 扩容因子为 2
        prompt = (char*)calloc(prompt_len + 1, sizeof(char));
        if (prompt == NULL) {
          shell.destructor(&shell);
          user.destructor(&user);
          throw_error("prompt", "reallocate prompt string failure");
          exit(failed);
        }
      }
      sprintf(prompt, prompt_fmt, shell.active_usr->name, shell.active_usr->uid, shell.cwd);
    }
  }

  release_ptr(prompt);
  shell.destructor(&shell);
  user.destructor(&user);

  return 0;
}
