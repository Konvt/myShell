#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "UserInfo.h"
#include "ShellUtilities.h"

int main()
{
  int commandLimit = 200;
  UserMgr mgr;
  if (CreateUserMgr(&mgr, commandLimit) == NULL)
    exit(2);

  const char *format = "%s@%x:%s$ ";
  int basicLen = mgr.nameLimit + UID_LEN + strlen(format);

  mgr.Login(&mgr);
  int cwdLen = strlen(mgr.cwd), promptLen = basicLen + cwdLen;
  char *prompt = (char*)calloc(promptLen + 1, sizeof(char));
  sprintf(prompt, format, mgr.hostName, mgr.uid, mgr.cwd); // splicing prompt

  while (1) {
    mgr.cmdMgr.Scanner(&mgr.cmdMgr, stdin, prompt)->Paraser(&mgr.cmdMgr, " ")->Processor(&mgr.cmdMgr);
    if (mgr.cmdMgr.lastCmdType == buildinCd) {
      mgr.Update(&mgr);
      cwdLen = strlen(mgr.cwd);
      /* ensure that the cwd can always be loaded into prompt */
      if (promptLen < basicLen + cwdLen) {
        /* if it is too small, an array expansion will be triggered
         * and make it as big as possible */
        promptLen = basicLen + (cwdLen * 2);
        free(prompt); prompt = (char*)calloc(promptLen + 1, sizeof(char));
      }
      sprintf(prompt, format, mgr.hostName, mgr.uid, mgr.cwd); // splicing prompt
    }
  }

  free(prompt);
  mgr.Destructor(&mgr);

  return 0;
}
