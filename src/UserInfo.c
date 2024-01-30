#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <unistd.h>

#include "UserInfo.h"
#include "CommandManager.h"
#include "ShellUtilities.h"

static int userNum = 0; // only functions in this file are allowed access

UserMgr* CreateUserMgr(UserMgr* const this, int readLimit)
{ // constructor
  if (readLimit <= 0) {
    ThrowError("The upper limit for reading must be greater than zero");
    return NULL;
  }

  userNum++;
  this->nameLimit = 32; // same as linux
  this->homeWd = this->cwd = NULL;
  this->hostName = (char*)calloc(this->nameLimit + 1, sizeof(char));
  if (this->hostName == NULL || CreateArgsMgr(&this->cmdMgr, readLimit) == NULL) {
    GarbagCollect(this->hostName);
    return NULL;
  }

  this->Destructor = &RecycleUserMgr;
  this->Login = &UserLogin;
  this->Update = &UpdateCwd;

  return this;
}

void RecycleUserMgr(UserMgr* const this)
{ // destructor
  GarbagCollect(this->cwd);
  GarbagCollect(this->hostName);
  GarbagCollect(this->homeWd);
  this->cmdMgr.Destructor(&this->cmdMgr);
  this->nameLimit = this->uid = -1;
  userNum--;
}

UserMgr* UserLogin(UserMgr* const this)
{
  char *prompt = "Login as: ";
  while (GetHostName(this->hostName, this->nameLimit, prompt) != SUCCESS)
    printf("\tWhitespace characters are not allowed, "
           "and characters are capped at %d\n\n", this->nameLimit);

  this->uid = GetID();
  this->homeWd = (char*)malloc(strlen(this->hostName) + 1);
  strcpy(this->homeWd, this->hostName); // set the home directory to the user name
  if (access(this->homeWd, F_OK) == -1) {
    if (MakeDir(&this->hostName, 1) == FAILED) {
      const char *errorFormat = "set dir '%s' failed!";
      char *tempPrompt = (char*)malloc(sizeof(char) * (strlen(errorFormat) + this->nameLimit + 1));
      sprintf(tempPrompt, errorFormat, this->hostName);

      ThrowError(tempPrompt);
      GarbagCollect(tempPrompt);
      exit(MKDIR_ERROR);
    }
  }
  ChangeCwd(this->homeWd);
  this->Update(this);
  if (this->cwd == NULL) {
    ThrowError("Get current dir failed!");
    exit(GET_CWD_ERROR);
  }

  Welcome();
  return this;
}

void UpdateCwd(UserMgr* const this)
{
  GarbagCollect(this->cwd);
  this->cwd = GetCwd(this->homeWd);
}

long GetID()
{
  return time(NULL);
}

int GetHostName(char* const name, int nameLimit, const char *prompt)
{
  char *input = (char*)calloc(nameLimit + 1, sizeof(char));
  GetInput(stdin, input, nameLimit, prompt);

  int hasSpace = FALSE, nameLen = 0;
  for (int i = 0; i < nameLimit; i++) {
    if (isspace(input[i])) {
      hasSpace = TRUE;
      break;
    } else if (input[i] != BLANK)
      nameLen++;
  }

  if (hasSpace == FALSE)
    strcpy(name, input);
  else if (hasSpace == TRUE || nameLen == 0) { // space characters isn't allowed
    const char *format = "\nUser name error: '%s'\n";
    char *info = (char*)malloc(sizeof(char) * (strlen(format) + strlen(input) + 1));
    sprintf(info, format, input);
    ThrowError(info);
    GarbagCollect(info);
  }

  GarbagCollect(input);
  return hasSpace == FALSE && nameLen != 0 ? SUCCESS : FAILED;
}
