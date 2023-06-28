#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "ShellUtilities.h"
#include "CommandManager.h"

ArgsMgr* CreateArgsMgr(ArgsMgr* const this, int readLimit)
{ // constructor
    if (readLimit <= 0) {
        ThrowError("The upper limit for reading must be greater than zero");
        exit(CONSTR_ERROR);
    }
    if (this == NULL) // always guarantees to return an object
        return CreateArgsMgr((ArgsMgr*)malloc(sizeof(ArgsMgr)), readLimit);

    this->_readLimit = readLimit;
    this->_commands = (char*)malloc(sizeof(char)*(readLimit+1));
    memset(this->_commands, '\0', sizeof(char)*(readLimit+1));
    this->_originalCmd = (char*)malloc(sizeof(char)*(readLimit+1));
    memset(this->_originalCmd, '\0', sizeof(char)*(readLimit+1));
    this->argc = this->_commandsLen = 0;
    this->args = NULL;

    this->Destructor = &RecycleArgsMgr;
    this->Scanner = &ReadCommand;
    this->Paraser = &CommandAnalyzer;
    this->Processor = &CommandProcess;

    return this;
}

void RecycleArgsMgr(ArgsMgr* const this)
{ // destructor
    free(this->args);
    free(this->_originalCmd);
    free(this->_commands);
    this->argc = this->_commandsLen = this->_readLimit = -1;
}

ArgsMgr* ReadCommand(ArgsMgr* const this, FILE* src, const char* prompt)
{
    GetInput(stdin, this->_commands, this->_readLimit, prompt);
    this->_commandsLen = strlen(this->_commands);
    /* clear the last saved command */
    memset(this->_originalCmd, '\0', this->_readLimit+1);
    strcpy(this->_originalCmd, this->_commands);

    return this;
}

ArgsMgr* CommandAnalyzer(ArgsMgr* const this, const char* delims)
{
    this->argc = 0;
    /* cut tokens by space */
    for (char *str=strtok(this->_commands, delims); str!=NULL; 
        str=strtok(NULL, delims), this->argc++);

    free(this->args);
    this->args = (char**)malloc(sizeof(char*)*this->argc+1);
    memset(this->args, 0, sizeof(char*)*this->argc);
    /* to meet the parameter requirements of execvp */
    this->args[this->argc] = NULL;

    /* find the token and assign their starting address to args */
    for (int i=1, backtrack=1, savedArgsNum=0; 
        i<=(this->_commandsLen) && savedArgsNum<(this->argc); i++, backtrack++) {
        if (this->_commands[i]!='\0' && this->_commands[i-1]=='\0')
            backtrack = 0; // backtrack is used to backtrack to the head of each token
        else if (this->_commands[i]=='\0' && this->_commands[i-1]!='\0') {
            this->args[savedArgsNum++] = &this->_commands[i-backtrack];
            backtrack = 0;
        }
    }

    return this;
}

CmdType CommandProcess(ArgsMgr* const this)
{
    if (this->args[0]==NULL || this->args[0][0]=='\0')
        return nill; // if command is a line of space
    CmdType type = nill;
    int executeFlag = FAILED, pipeFlag = -1, reFlag = -1;

    for (int i=0; i<this->argc; i++) {
        if (strcmp(this->args[i], "||") == 0)
            pipeFlag = i;
        else if (strcmp(this->args[i], ">>") == 0)
            reFlag = i;
    }
    /* they are mutually exclusive */
    if (pipeFlag!=-1 && reFlag!=-1) {
        ThrowError("Unsupported command");
        return nill;
    }

    if (pipeFlag != -1)
        executeFlag = PipeExecute(this->args, this->argc, pipeFlag);
    else if (reFlag != -1)
        executeFlag = RedirectExecute(this->args, this->argc, reFlag);
    else {
        type = SyntacticParser(this->args, this->argc);
        if (type > buildin)
            executeFlag = BuildinCmds(this->args, this->argc, type);
        else if (type < buildin)
            executeFlag = ExternalCmds(this->args, this->argc, type);
    }

    if (executeFlag == FAILED) {
        ThrowError(this->_originalCmd);
        puts("command error or not found\n");
    }

    return type;
}

int PipeExecute(char** args, int argc, int pipeFlag)
{
    if (argc<3 || pipeFlag==0 || pipeFlag==argc-1)
        return FAILED;
    /* transfer the output of the left side of '||' to the stdin of the right */
    int streamPipe[2], statusPipe[2];
    if (pipe(streamPipe) == -1)
        return FAILED;
    if (pipe(statusPipe) == -1)
        return FAILED;
    fcntl(statusPipe[0], F_SETFL, fcntl(statusPipe[0], F_GETFL)|O_NONBLOCK);
    /* command with no output will cause one of the subprocesses to block continuously */
    /* so a non-blocking pipe is used here */
    fcntl(streamPipe[0], F_SETFL, fcntl(streamPipe[0], F_GETFL)|O_NONBLOCK);

    CmdType type[2] = { nill, nill };
    int executeFlag = SUCCESS;
    type[0] = SyntacticParser(args, argc);
    type[1] = SyntacticParser(args+pipeFlag+1, argc-pipeFlag-1);

    pid_t pid[2];
    for (int cmdNum=0; cmdNum<2; cmdNum++) {
        /* all commands will be executed after forking */
        /* thus the commands such as "exit" will not be executed actually */
        pid[cmdNum] = fork();
        if (pid[cmdNum] == 0) {
            close(statusPipe[0]);
            /* in front of '||' */
            if (cmdNum == 0) {
                close(streamPipe[0]);
                /* 1 is stdout */
                dup2(streamPipe[1], 1);
                /* reorganizes args and argc that the current process should process */
                args[pipeFlag] = NULL;
                argc = pipeFlag;
            /* at the back of '||' */
            } else if (cmdNum == 1) {
                close(streamPipe[1]);
                /* 0 is stdin */
                dup2(streamPipe[0], 0);
                args = args+pipeFlag+1;
                argc = argc-pipeFlag-1;
            }
            if (type[cmdNum] > buildin)
                executeFlag = BuildinCmds(args, argc, type[cmdNum]);
            else {
                executeFlag = ExternalCmds(args, argc, type[cmdNum]);
                write(statusPipe[1], &executeFlag, sizeof(int));
            }
            close(streamPipe[0]);
            close(streamPipe[1]);
            exit(SUBPROCESS_EXIT);
        }
        /* there will be nothing to read if we don't wait for subprocess end */
        waitpid(pid[cmdNum], NULL, 0);
    }
    read(statusPipe[0], &executeFlag, sizeof(int));

    close(streamPipe[0]); close(streamPipe[1]);
    close(statusPipe[0]); close(statusPipe[1]);
    return executeFlag;
}

int RedirectExecute(char** args, int argc, int reFlag)
{
    if (argc-reFlag-1 > 1) {
        ThrowError("Too many files");
        return FAILED;
    }
    if (access(args[reFlag+1], F_OK) == -1) {
        if (TouchFile(args+reFlag+1, 1)) {
            ThrowError("Create target file failed");
            return FAILED;
        }
    } else if (access(args[reFlag+1], W_OK) == -1) {
        ThrowError("Target file cannot be written");
        return FAILED;
    }

    int fd[2], executeFlag = SUCCESS;
    if (pipe(fd) == -1)
        return FAILED;
    fcntl(fd[0], F_SETFL, fcntl(fd[0], F_GETFL)|O_NONBLOCK);

    CmdType type = SyntacticParser(args, argc);
    int output = open(args[reFlag+1], O_WRONLY);
    if (fork() == 0) {
        /* redirect the stdout to the file 'output' */
        dup2(output, 1);
        close(fd[0]);
        args[reFlag] = NULL;
        argc = argc-reFlag-1;
        if (type > buildin)
            executeFlag = BuildinCmds(args, argc, type);
        else {
            executeFlag = ExternalCmds(args, argc, type);
            write(fd[1], &executeFlag, sizeof(int));
        }
        close(fd[1]);
        exit(SUBPROCESS_EXIT);
    }
    wait(NULL);
    read(fd[0], &executeFlag, sizeof(int));

    close(output);
    close(fd[0]);
    close(fd[1]);
    return executeFlag;
}

int BuildinCmds(char** args, int argc, CmdType type)
{ /* args isn't const-ness because const can be avoided in some ways */
    switch (type) {
    case buildinExit:
        exit(EXIT_SUCCESS);
    case buildinHelp:
        GetHelp();
        return SUCCESS;
    case buildinEcho:
        return Echo(args, argc);
    case buildinPwd:
        char *temp = GetCwd(NULL);
        puts(temp); free(temp);
        return SUCCESS;
    case buildinCd:
        if (argc > 2) {
            ThrowError("cd: too many arguments");
            return FAILED;
        }
        return ChangeCwd(args[1]);
    case gotoMoon:
        return GotoTheMoon();
    case fiveFives:
        return FiveFives();
    default:
        break;
    }

    return FAILED;
}

int ExternalCmds(char** args, int argc, CmdType type)
{
    switch (type) {
    case externCopy:
        return CopyFile(args, argc);
    case externMkdir:
        return MakeDir(args, argc);
    case externTouch:
        return TouchFile(args, argc);
    case externLook:
        return LookFile(args, argc);
    case externLs:
        return ListDir(args, argc);
    case external:
    default:
        return Execute(args, argc);
    }

    return FAILED;
}

CmdType SyntacticParser(char** args, int argc)
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
        if (strcmp(args[0], "ls")==0 || strcmp(args[0], "dir")==0)
            return externLs;
        else if (strcmp(args[0], "look") == 0)
            return externLook;
        break;
    case 'g':
        if (argc==2 && strcmp(args[0], "goto")==0 
            && strcmp(args[1], "moon")==0)
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
