#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "string_util.h"
#include "error_handle.h"
#include "functional_func.h"

#include "constant_def.h"
#include "font_style.h"

#define DEFAULT_MODE 0777

void get_help()
{
  puts(
    "\n'exit' to exit\n'help' to get help\n"
    "'echo <string>' to print string to stdout\n"
    "'pwd' to provide working address dir\n"
    "'cd' to change dir\n'cp <srouce> <target>' to copy file\n"
     "'mkdir <dir list>' to make new dirs\n'touch <file list>' to make new files\n"
     "'look <file list>' to view the files as text\n"
     "'ls' or 'dir' to list current dir, WITHOUT ARGUMENTS\n"
     "'cls' to clear up the screen\n"
     "'<command> | <command>' to use pipe function\n"
     "'<command>' > '<file>' to redirect output stream\n"
     "'goto moon' is something interesting\n"
     "\n\t>>> A singal command each line, pipe and redirect excepted <<<\n"
  );
}

char* get_cwd(const char* home)
{
  char *cwd = getcwd(NULL, 0);

  if (home != NULL)
    format_str(cwd, home);
  return cwd;
}

int change_wd(const char* target)
{
  if (access(target, F_OK) != -1)
    return chdir(target) == 0 ? success : failed;
  else return failed;
}

int echo(char** args, int argc)
{
  if (argc <= 1) return failed;
  for (int i = 1; i < argc; i++)
    puts(args[i]);

  return success;
}

int make_dir(char **args, int argc)
{
  int flag = 0;
  if (argc == 1)
    flag = mkdir(args[0], DEFAULT_MODE);
  else {
    for (int i = 1; i < argc; i++)
      flag = mkdir(args[i], DEFAULT_MODE);
  }

  return flag != -1 ? success : failed;
}

int touch_file(char **args, int argc)
{
  int flag = 0;
  if (argc == 1)
    flag = creat(args[0], DEFAULT_MODE);
  else {
    for (int i = 1; i < argc; i++)
      flag = creat(args[i], DEFAULT_MODE);
  }

  return flag != -1 ? success : failed;
}

int look_file(char **args, int argc)
{
  if (argc <= 1) return failed;
  for (int i = 1; i < argc; i++) {
    if (access(args[i], F_OK | R_OK) == -1)
      throw_error("look", "file don't exist or cannot be read");
    else {
      int buf_size = 1025;
      FILE *src = fopen(args[i], "r");
      char *buffer = (char*)malloc(sizeof(char) * buf_size);
      do {
        fgets(buffer, buf_size, src);
        fputs(buffer, stdout);
      } while (feof(src) == 0);
      fclose(src);
    }
  }

  return success;
}

int list_dir(char **args, int argc)
{
  if (argc > 1) {
    throw_error("ls", "too many arguments");
    return failed;
  }
  DIR *dir_ptr = opendir(".");
  struct dirent *ent = NULL;

  while ((ent = readdir(dir_ptr)) != NULL) {
    if ((ent->d_type == 4 || ent->d_type == 8)
        && ent->d_name[0] != '.')
      printf("%s  ", ent->d_name);
  }
  closedir(dir_ptr);
  putchar('\n');

  return success;
}

int copy_file(char **args, int argc)
{
  /* parameter error or source file does not exist */
  if (argc < 2 || access(args[1], F_OK) == -1) {
    if (argc >= 2) throw_error("cp", "source file don't exist");
    else throw_error("cp", "too many arguments");
    return failed;
  } else if (access(args[1], R_OK) == -1) {
    throw_error("cp", "source file cannot be read");
    return failed;
  }

  struct stat file_attr;
  stat(args[1], &file_attr);
  if (access(args[2], F_OK) == -1) {
    if (touch_file(args + 2, 1) == failed) {
      throw_error("cp", "create target file failed");
      return failed;
    }
  }
  if (access(args[2], W_OK) == -1) {
    throw_error("cp", "target file cannot be written");
    return failed;
  }

  int buf_size = 512;
  if (file_attr.st_size >= 10240)
    buf_size = 10240;
  else if (file_attr.st_size >= 1024)
    buf_size = 1024;

  /* C lib functions have their own buffers, so we don't use system call here */
  FILE *src_file = fopen(args[1], "rb"), *dst_file = fopen(args[2], "wb");
  char *buffer = (char*)malloc(sizeof(char) * buf_size);
  do {
    /* if the last line is small than buffer, some null characters will be input to the file */
    int readNum = fread(buffer, sizeof(char), buf_size, src_file);
    fwrite(buffer, sizeof(char), readNum, dst_file);
  } while (feof(src_file) == 0);

  fclose(src_file);
  fclose(dst_file);
  release_ptr(buffer);
  return success;
}

int execute(char **args, int argc)
{
  int fd[2], status = success;
  if (pipe(fd) == -1) {
    throw_error("execute", "cannot create pipe");
    return failed;
  }
  /* function execute may not return, which means there may not be anything in pipe *
   * so we need to use a non-blocking pipe again */
  fcntl(fd[0], F_SETFL, fcntl(fd[0], F_GETFL) | O_NONBLOCK);

  pid_t pid = fork();
  if (pid < 0) {
    throw_error("execute", "cannot fork");
    return failed;
  }
  if (pid == 0) {
    close(fd[0]);
    execvp(args[0], args + 1);
    /* if failed */
    int message = failed;
    write(fd[1], &message, sizeof(int));
    exit(failed);
  } else {
    waitpid(pid, NULL, 0);
    read(fd[0], &status, sizeof(int));
  }

  return status;
}

int clean_up(char **args, int argc)
{
  if (argc > 1) {
    throw_error("cls", "too many arguments");
    return failed;
  }
  fputs(FNT_RESET_CURSOR FNT_CLS, stdout);
  return success;
}

int goto_moon()
{
  puts(
    FNT_BLUE
    "\n▄████  █    ▀▄    ▄     █▀▄▀█ ▄███▄          ▄▄▄▄▀ ████▄        ▄▄▄▄▀ ▄  █ ▄███▄       █▀▄▀█ ████▄ ████▄    ▄   \n"
      "█▀   ▀ █      █  █      █ █ █ █▀   ▀      ▀▀▀ █    █   █     ▀▀▀ █   █   █ █▀   ▀      █ █ █ █   █ █   █     █  \n"
      "█▀▀    █       ▀█       █ ▄ █ ██▄▄            █    █   █         █   ██▀▀█ ██▄▄        █ ▄ █ █   █ █   █ ██   █ \n"
      "█      ███▄    █        █   █ █▄   ▄▀        █     ▀████        █    █   █ █▄   ▄▀     █   █ ▀████ ▀████ █ █  █ \n"
      " █         ▀ ▄▀            █  ▀███▀         ▀                  ▀        █  ▀███▀          █              █  █ █ \n"
      "  ▀                       ▀                                            ▀                 ▀               █   ██ \n"
    FNT_RESET  
  );

  return success;
}

int five_fives()
{
  puts(
    FNT_RED
    "\n▓██   ██▓ ▒█████   █    ██  ██▀███      ██▓    ▄▄▄        ██████ ▄▄▄█████▓     █████▒██▓ ██▀███    ██████ ▄▄▄█████▓   ▓█████▄  ▄▄▄     ▓██   ██▓\n"
    " ▒██  ██▒▒██▒  ██▒ ██  ▓██▒▓██ ▒ ██▒   ▓██▒   ▒████▄    ▒██    ▒ ▓  ██▒ ▓▒   ▓██   ▒▓██▒▓██ ▒ ██▒▒██    ▒ ▓  ██▒ ▓▒   ▒██▀ ██▌▒████▄    ▒██  ██▒\n"
    "  ▒██ ██░▒██░  ██▒▓██  ▒██░▓██ ░▄█ ▒   ▒██░   ▒██  ▀█▄  ░ ▓██▄   ▒ ▓██░ ▒░   ▒████ ░▒██▒▓██ ░▄█ ▒░ ▓██▄   ▒ ▓██░ ▒░   ░██   █▌▒██  ▀█▄   ▒██ ██░\n"
    "  ░ ▐██▓░▒██   ██░▓▓█  ░██░▒██▀▀█▄     ▒██░   ░██▄▄▄▄██   ▒   ██▒░ ▓██▓ ░    ░▓█▒  ░░██░▒██▀▀█▄    ▒   ██▒░ ▓██▓ ░    ░▓█▄   ▌░██▄▄▄▄██  ░ ▐██▓░\n"
    "  ░ ██▒▓░░ ████▓▒░▒▒█████▓ ░██▓ ▒██▒   ░██████▒▓█   ▓██▒▒██████▒▒  ▒██▒ ░    ░▒█░   ░██░░██▓ ▒██▒▒██████▒▒  ▒██▒ ░    ░▒████▓  ▓█   ▓██▒ ░ ██▒▓░\n"
    "   ██▒▒▒ ░ ▒░▒░▒░ ░▒▓▒ ▒ ▒ ░ ▒▓ ░▒▓░   ░ ▒░▓  ░▒▒   ▓▒█░▒ ▒▓▒ ▒ ░  ▒ ░░       ▒ ░   ░▓  ░ ▒▓ ░▒▓░▒ ▒▓▒ ▒ ░  ▒ ░░       ▒▒▓  ▒  ▒▒   ▓▒█░  ██▒▒▒ \n"
    " ▓██ ░▒░   ░ ▒ ▒░ ░░▒░ ░ ░   ░▒ ░ ▒░   ░ ░ ▒  ░ ▒   ▒▒ ░░ ░▒  ░ ░    ░        ░      ▒ ░  ░▒ ░ ▒░░ ░▒  ░ ░    ░        ░ ▒  ▒   ▒   ▒▒ ░▓██ ░▒░ \n"
    " ▒ ▒ ░░  ░ ░ ░ ▒   ░░░ ░ ░   ░░   ░      ░ ░    ░   ▒   ░  ░  ░    ░          ░ ░    ▒ ░  ░░   ░ ░  ░  ░    ░          ░ ░  ░   ░   ▒   ▒ ▒ ░░  \n"
    " ░ ░         ░ ░     ░        ░            ░  ░     ░  ░      ░                      ░     ░           ░                 ░          ░  ░░ ░     \n"
    "░ ░                                                                                                                   ░                ░ ░     \n"
    FNT_RESET
  );

  return success;
}
