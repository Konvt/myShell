#ifndef __SEL_FUNCTION_FUNC__
  #define __SEL_FUNCTION_FUNC__

void welcome();

void get_help();

/* gets cwd, if `home` is NULL, returns an unprocessed string */
char* get_cwd(const char* home);

int change_wd(const char* target);

int echo(char** args, int argc);

int make_dir(char **args, int argc);

int touch_file(char **args, int argc);

/* look at the files */
int look_file(char **args, int argc);

/* only supports 'ls' that without arguments */
int list_dir(char **args, int argc);

int copy_file(char **args, int argc);

int execute(char **args, int argc);

int clean_up(char **args, int argc);

/* something interesting */
int goto_moon();
int five_fives();

#endif
