#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "UserInfo.h"
#include "ShellUtilities.h"

int main(int argc, char **argv)
{
    int commandLimit = 200;
    UserMgr mgr; CreateUserMgr(&mgr, commandLimit);

    const char *format = "%s@%x:%s$ ";
    int basicLen = mgr.nameLimit+UID_LEN+strlen(format);

    mgr.Login(&mgr);
    int cwdLen = strlen(mgr.cwd), promptLen = basicLen+cwdLen;
    char *prompt = (char*)malloc(sizeof(char)*(promptLen+1));
    while (1) {
        /* prompt will be re-spliced only after the cwd has been updated */
        if (MatchSubstr(prompt, mgr.cwd, 0) == FAILED) {
            memset(prompt, '\0', sizeof(char)*(promptLen+1));
            sprintf(prompt, format, mgr.hostName, mgr.uid, mgr.cwd); // splicing prompt
        }

        mgr.cmdMgr.Scanner(&mgr.cmdMgr, stdin, prompt)->Paraser(&mgr.cmdMgr, " ");
        mgr.lastCmdType = mgr.cmdMgr.Processor(&mgr.cmdMgr);
        if (mgr.lastCmdType == buildinCd)
            mgr.Update(&mgr);

        /* ensure that the cwd can always be loaded into prompt */
        cwdLen = strlen(mgr.cwd);
        if (promptLen < basicLen+cwdLen) {
            /* if it is too small, an array expansion will be triggered */
            /* and make it as big as possible */
            promptLen = basicLen+(cwdLen*2);
            free(prompt);
            prompt = (char*)malloc(sizeof(char)*(promptLen+1));
        }
    }

    free(prompt);
    return 0;
}
