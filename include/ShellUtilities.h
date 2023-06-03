#ifndef __SHELL_H_
    #define __SHELL_H_
#include <stdio.h>

/* error code */
#define SUBPROCESS_EXIT 0
#define GET_CWD_ERROR 1
#define CONSTR_ERROR 2
#define MKDIR_ERROR 3

#define TRUE 1
#define FALSE 0
#define SUCCESS 0
#define FAILED -1

#define EGG TRUE

/* throw a line of error info to stdout */
void ThrowError(const char* what);
/* terminus is the hypertail offset, return is a pointer to the first non-null character */
char* StrTrim(char* const str, const size_t terminus);
/* inputLimit is the num of non-null characters, this function is called for all input requests */
void GetInput(FILE* src, char* acceptInput, int inputLimit, const char* prompt);
/* matches substr from pos, return the starting offset in mainstr, or -1 if it is not found */
int MatchSubstr(const char* mainstr, const char* substr, int pos);
/* process the string get from GetCwd() */
void AdjustDir(char* wd, const char* homeWd);

/* commands function */
void Welcome();
void GetHelp();
/* gets cwd, if pass an empty argument, returns an unprocessed string */
char* GetCwd(const char* homeWd);
int ChangeCwd(const char* target);
/* only the execution status of the last parameter will be returned */
int Echo(char** args, int argc);
int MakeDir(char **args, int argc);
/* ditto */
int TouchFile(char **args, int argc);
/* look at the files */
int LookFile(char **args, int argc);
/* only support ls that without arguments */
int ListDir(char **args, int argc);
int CopyFile(char **args, int argc);
int Execute(char **args, int argc);
/* something interesting */
int GotoTheMoon();
int FiveFives();

#endif
