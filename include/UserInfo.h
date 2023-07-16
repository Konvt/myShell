#ifndef __USERINFO_H_
    #define __USERINFO_H_
#include "CommandManager.h"

#define UID_LEN 8 // the string length of a hexadecimal number of long type

typedef struct UserInfoManager_t {
    int nameLimit;
    long uid;
    CmdType lastCmdType;
    char *cwd, *hostName, *homeWd;
    ArgsMgr cmdMgr;

    void (*Destructor)(struct UserInfoManager_t* const);
    struct UserInfoManager_t* (*Login)(struct UserInfoManager_t* const);
    void (*Update)(struct UserInfoManager_t* const);
} UserMgr;

UserMgr* CreateUserMgr(UserMgr* const this, int readLimit);
void RecycleUserMgr(UserMgr* const this);
UserMgr* UserLogin(UserMgr* const this);
void UpdateCwd(UserMgr* const this);

long GetID();
/* if the user name is valid, return 0, else return -1 */
int GetHostName(char* const name, int nameLimit, const char *prompt);

#endif
