#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "ShellUtilities.h"

#define DEFAULT_MODE 0777

void Welcome()
{
    puts("\n _       __     __                             __                           _____ __         ____\n"
        "| |     / /__  / /________  ____ ___  ___     / /_____     ____ ___  __  __/ ___// /_  ___  / / /\n"
        "| | /| / / _ \\/ / ___/ __ \\/ __ `__ \\/ _ \\   / __/ __ \\   / __ `__ \\/ / / /\\__ \\/ __ \\/ _ \\/ / / \n"
        "| |/ |/ /  __/ / /__/ /_/ / / / / / /  __/  / /_/ /_/ /  / / / / / / /_/ /___/ / / / /  __/ / /  \n"
        "|__/|__/\\___/_/\\___/\\____/_/ /_/ /_/\\___/   \\__/\\____/  /_/ /_/ /_/\\__, //____/_/ /_/\\___/_/_/   \n"
        "                                                                  /____/                         \n"
        );
}

void GetHelp()
{
    puts("\n'exit' to exit\n'help' to get help\n"
        "'echo <string>' to print string to stdout\n"
        "'pwd' to provide working address dir\n"
        "'cd' to change dir\n'cp <srouce> <target>' to copy file\n"
        "'mkdir <dir list>' to make new dirs\n'touch <file list>' to make new files\n"
        "'look <file list>' to view the files as text\n"
        "'ls' or 'dir' to list current dir, WITHOUT ARGUMENTS\n"
        "'<command> || <command>' to use pipe function\n"
        "'<command>' >> '<file>' to redirect output stream\n"
        "'goto moon' is something interesting\n"
        "\n\t>>> A singal command each line, pipe and redirect excepted <<<\n");
}

char* GetCwd(const char* homeWd)
{
    char *cwd = getcwd(NULL, 0);

    if (homeWd != NULL)
        AdjustDir(cwd, homeWd);
    return cwd;
}

int ChangeCwd(const char* target)
{
    if (access(target, F_OK) != -1)
        return chdir(target)==0 ? SUCCESS : FALSE;
    else return FAILED;
}

int Echo(char** args, int argc)
{
    if (argc <= 1) return FAILED;
    for (int i=1; i<argc; i++)
        fputs(args[i], stdout);
    putchar('\n');

    return SUCCESS;
}

int MakeDir(char **args, int argc)
{
    int flag = 0;
    if (argc == 1)
        flag = mkdir(args[0], DEFAULT_MODE);
    else {
        for (int i=1; i<argc; i++)
            flag = mkdir(args[i], DEFAULT_MODE);
    }

    return flag!=-1 ? SUCCESS : FAILED;
}

int TouchFile(char **args, int argc)
{
    int flag = 0;
    if (argc == 1)
        flag = creat(args[0], DEFAULT_MODE);
    else {
        for (int i=1; i<argc; i++)
            flag = creat(args[i], DEFAULT_MODE);
    }

    return flag!=-1 ? SUCCESS : FAILED;
}

int LookFile(char **args, int argc)
{
    if (argc <= 1) return FAILED;
    for (int i=1; i<argc; i++) {
        if (access(args[i], F_OK|R_OK) == -1)
            ThrowError("File don't exist or cannot be read");
        else {
            int bufSize = 1025;
            FILE *src = fopen(args[i], "r");
            char *buffer = (char*)malloc(sizeof(char)*bufSize);
            do {
                fgets(buffer, bufSize, src);
                fputs(buffer, stdout);
            } while (feof(src) == 0);
            fclose(src);
        }
    }
}

int ListDir(char **args, int argc)
{
    /* not support arguments ls commands */
    if (argc > 1) {
        ThrowError("ls: too many arguments");
        return FAILED;
    }
    DIR *dirPtr = opendir(".");
    struct dirent *ent = NULL;

    while((ent = readdir(dirPtr)) != NULL) {
        if ((ent->d_type==4 || ent->d_type==8) 
            && ent->d_name[0]!='.')
            printf("%s  ", ent->d_name);
    }
    closedir(dirPtr);
    putchar('\n');

    return SUCCESS;
}

int CopyFile(char **args, int argc)
{
    /* parameter error or source file does not exist */
    if (argc<2 || access(args[1], F_OK)==-1) {
        if (argc >= 2) ThrowError("Source file don't exist");
        else ThrowError("cp: Too many arguments");
        return FAILED;
    } else if (access(args[1], R_OK) == -1) {
        ThrowError("Source file cannot be read");
        return FAILED;
    }

    struct stat fileAttr;
    stat(args[1], &fileAttr);
    if (access(args[2], F_OK) == -1) {
        if (TouchFile(args+2, 1) == FAILED) {
            ThrowError("Create target file failed");
            return FAILED;
        }
    }
    if (access(args[2], W_OK) == -1) {
        ThrowError("Target file cannot be written");
        return FAILED;
    }

    int bufSize = 512;
    if (fileAttr.st_size >= 10240)
        bufSize = 10240;
    else if (fileAttr.st_size >= 1024)
        bufSize = 1024;

    /* C lib functions have their own buffers, so we don't use system call here */
    FILE *srcFile = fopen(args[1], "rb"), *dstFile = fopen(args[2], "wb");
    char *buffer = (char*)malloc(sizeof(char)*bufSize);
    do {
        /* if the last line is small than buffer, some null characters will be input to the file */
        int readNum = fread(buffer, sizeof(char), bufSize, srcFile);
        fwrite(buffer, sizeof(char), readNum, dstFile);
    } while (feof(srcFile) == 0);

    fclose(srcFile);
    fclose(dstFile);
    free(buffer);
    return SUCCESS;
}

int Execute(char **args, int argc)
{
    int fd[2], status = SUCCESS;
    if (pipe(fd) == -1)
        return FAILED;
    /* execute may not return, which means there may not be anything in pipe */
    /* so we need to use a non-blocking pipe */
    fcntl(fd[0], F_SETFL, fcntl(fd[0], F_GETFL)|O_NONBLOCK);

    pid_t pid = fork();
    if (pid < 0) {
        ThrowError("Execute error: cannot fork");
        return FAILED;
    }
    if (pid == 0) {
        close(fd[0]);
        execvp(args[0], args+1);
        /* if failed */
        int message = FAILED;
        write(fd[1], &message, sizeof(int));
        exit(FAILED);
    } else {
        waitpid(pid, NULL, 0);
        read(fd[0], &status, sizeof(int));
    }

    return status;
}

int GotoTheMoon()
{
#if EGG
    puts("\n▄████  █    ▀▄    ▄     █▀▄▀█ ▄███▄          ▄▄▄▄▀ ████▄        ▄▄▄▄▀ ▄  █ ▄███▄       █▀▄▀█ ████▄ ████▄    ▄   \n"
        "█▀   ▀ █      █  █      █ █ █ █▀   ▀      ▀▀▀ █    █   █     ▀▀▀ █   █   █ █▀   ▀      █ █ █ █   █ █   █     █  \n"
        "█▀▀    █       ▀█       █ ▄ █ ██▄▄            █    █   █         █   ██▀▀█ ██▄▄        █ ▄ █ █   █ █   █ ██   █ \n"
        "█      ███▄    █        █   █ █▄   ▄▀        █     ▀████        █    █   █ █▄   ▄▀     █   █ ▀████ ▀████ █ █  █ \n"
        " █         ▀ ▄▀            █  ▀███▀         ▀                  ▀        █  ▀███▀          █              █  █ █ \n"
        "  ▀                       ▀                                            ▀                 ▀               █   ██ \n");
#endif

    return SUCCESS;
}

int FiveFives()
{
#if EGG
    puts("\n▓██   ██▓ ▒█████   █    ██  ██▀███      ██▓    ▄▄▄        ██████ ▄▄▄█████▓     █████▒██▓ ██▀███    ██████ ▄▄▄█████▓   ▓█████▄  ▄▄▄     ▓██   ██▓\n"
        " ▒██  ██▒▒██▒  ██▒ ██  ▓██▒▓██ ▒ ██▒   ▓██▒   ▒████▄    ▒██    ▒ ▓  ██▒ ▓▒   ▓██   ▒▓██▒▓██ ▒ ██▒▒██    ▒ ▓  ██▒ ▓▒   ▒██▀ ██▌▒████▄    ▒██  ██▒\n"
        "  ▒██ ██░▒██░  ██▒▓██  ▒██░▓██ ░▄█ ▒   ▒██░   ▒██  ▀█▄  ░ ▓██▄   ▒ ▓██░ ▒░   ▒████ ░▒██▒▓██ ░▄█ ▒░ ▓██▄   ▒ ▓██░ ▒░   ░██   █▌▒██  ▀█▄   ▒██ ██░\n"
        "  ░ ▐██▓░▒██   ██░▓▓█  ░██░▒██▀▀█▄     ▒██░   ░██▄▄▄▄██   ▒   ██▒░ ▓██▓ ░    ░▓█▒  ░░██░▒██▀▀█▄    ▒   ██▒░ ▓██▓ ░    ░▓█▄   ▌░██▄▄▄▄██  ░ ▐██▓░\n"
        "  ░ ██▒▓░░ ████▓▒░▒▒█████▓ ░██▓ ▒██▒   ░██████▒▓█   ▓██▒▒██████▒▒  ▒██▒ ░    ░▒█░   ░██░░██▓ ▒██▒▒██████▒▒  ▒██▒ ░    ░▒████▓  ▓█   ▓██▒ ░ ██▒▓░\n"
        "   ██▒▒▒ ░ ▒░▒░▒░ ░▒▓▒ ▒ ▒ ░ ▒▓ ░▒▓░   ░ ▒░▓  ░▒▒   ▓▒█░▒ ▒▓▒ ▒ ░  ▒ ░░       ▒ ░   ░▓  ░ ▒▓ ░▒▓░▒ ▒▓▒ ▒ ░  ▒ ░░       ▒▒▓  ▒  ▒▒   ▓▒█░  ██▒▒▒ \n"
        " ▓██ ░▒░   ░ ▒ ▒░ ░░▒░ ░ ░   ░▒ ░ ▒░   ░ ░ ▒  ░ ▒   ▒▒ ░░ ░▒  ░ ░    ░        ░      ▒ ░  ░▒ ░ ▒░░ ░▒  ░ ░    ░        ░ ▒  ▒   ▒   ▒▒ ░▓██ ░▒░ \n"
        " ▒ ▒ ░░  ░ ░ ░ ▒   ░░░ ░ ░   ░░   ░      ░ ░    ░   ▒   ░  ░  ░    ░          ░ ░    ▒ ░  ░░   ░ ░  ░  ░    ░          ░ ░  ░   ░   ▒   ▒ ▒ ░░  \n"
        " ░ ░         ░ ░     ░        ░            ░  ░     ░  ░      ░                      ░     ░           ░                 ░          ░  ░░ ░     \n"
        "░ ░                                                                                                                   ░                ░ ░     \n");
#endif

    return SUCCESS;
}
