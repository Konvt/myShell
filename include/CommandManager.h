#ifndef __CMDMGR_H_
    #define __CMDMGR_H_
#include <stdio.h>

typedef enum CommandType_t {
/* the first member of each line is used as separator and has no real meaning */
    nil,
    external, externCopy, externMkdir, externTouch, externLook, externLs,
    buildin, buildinExit, buildinEcho, buildinHelp, buildinPwd, buildinCd,
    gotoMoon, fiveFives,
} CmdType;

typedef struct CommandManager_t {
// private:
    int _readLimit; // only records the length of non-null character
    int _commandsLen; // ditto
    char *_originalCmd;
    char *_commands;
// public:
    int argc;
    char **args;

    void (*Destructor)(struct CommandManager_t* const);
    struct CommandManager_t* (*Scanner)(struct CommandManager_t* const, FILE *, const char*);
    struct CommandManager_t* (*Paraser)(struct CommandManager_t* const, const char*);
    CmdType (*Processor)(struct CommandManager_t* const);
} ArgsMgr;

ArgsMgr* CreateArgsMgr(ArgsMgr* const this, int readLimit);
void RecycleArgsMgr(ArgsMgr* const this);

ArgsMgr* ReadCommand(ArgsMgr* const this, FILE* src, const char* prompt);
/* split all tokens in the command by space, and save their starting address of each token into args */
ArgsMgr* CommandAnalyzer(ArgsMgr* const this, const char* delims);
/* process the command, return is the type of command */
CmdType CommandProcess(ArgsMgr* const this);

/* pipe and redirect will be processed separately */
int PipeExecute(char** args, int argc, int pipeFlag);
int RedirectExecute(char** args, int argc, int reFlag);
int BuildinCmds(char** args, int argc, CmdType type);
int ExternalCmds(char** args, int argc, CmdType type);
CmdType SyntacticParser(char** args, int argc);

#endif
