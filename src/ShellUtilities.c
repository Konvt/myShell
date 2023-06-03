#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ShellUtilities.h"

void ThrowError(const char* what)
{
    printf("\nError due to: \n\t'%s'\n", what);
}

char* StrTrim(char* const str, const size_t terminus)
{
    if (str==NULL || terminus<=0)
        return NULL; // an error offset

    size_t index;
    /* unlimit string interception range by address */
    for (index=terminus-1; (str+index)>=str; index--) {
        if (isspace(str[index]))
            str[index] = '\0';
        else if (str[index] != '\0')
            break;
    }
    for (index=0; (str+index)<(str+terminus); index++) {
        if (isspace(str[index]))
            str[index] = '\0';
        else break;
    }

    return str+index;
}

void GetInput(FILE* src, char* acceptInput, int inputLimit, const char* prompt)
{
    if (acceptInput==NULL || inputLimit==0) return;
    char *haveTrimmed = NULL;

    fflush(src);
    acceptInput[0] = '\0';
    do {
        if (prompt!=NULL && acceptInput[0]=='\0')
            fputs(prompt, stdout);
        fgets(acceptInput, inputLimit, src);
        int len = strlen(acceptInput);
        haveTrimmed = StrTrim(acceptInput, len+1);
    } while (haveTrimmed[0] == '\0');

    /* if there are spaces at the beginning
     * some non-null characters will be stayed in the middle of the string
     * thus we need to move them to front */
    strcpy(acceptInput, haveTrimmed);
    /* make sure there are not left any unwanted characters behind */
    for (int i=0; ; i++) {
        if (acceptInput[i] == '\0') {
            memset(acceptInput+i+1, '\0', inputLimit-i-1);
            break;
        }
    }
}

int MatchSubstr(const char* mainstr, const char* substr, int pos)
{
    int subLen = strlen(substr);
    /* null character at the end of the string allows us
     * not to calculate the main string length */
    if (subLen <= 2) { // substr is too short to use KMP
        char *ret = strstr(mainstr+pos, substr);
        return ret==NULL ? -1 : (int)(ret-mainstr);
    }
    int *next = (int*)malloc(sizeof(int)*subLen);
    /* matches string by using KMP */

    int offset = 0, trace = -1;
    next[0] = -1;
    while (substr[offset] != '\0') {
        if(trace==-1 || substr[offset]==substr[trace])
            next[++offset] = ++trace;
        else trace = next[trace];
    }

    offset = pos, trace = 0;
    while (mainstr[offset]!='\0' && trace<subLen) {
        if (trace==-1 || mainstr[offset]==substr[trace]) {
            /* if we match a correct string, break */
            if (substr[trace+1] == '\0') break;
            ++offset; ++trace;
        }
        else trace = next[trace];
    }

    free(next);
    if (mainstr[offset] == '\0')
        return FAILED; // substr isn't in mainstr
    return offset-subLen+1;
}

void AdjustDir(char* wd, const char* homeWd)
{ /* this function will set the string like 'system/home/name' to be '~/name */
    const char sep = '/';
    int mainLen = strlen(wd), subLen = strlen(homeWd);

    int pos = 0, exitFlag = FALSE;
    do {
        int offset = MatchSubstr(wd, homeWd, pos);
        if (offset-1>=0 && wd[offset-1]!=sep) {
            /* when we match something like 'byhome/'
             * it's an error match, rematches with a new pos */
            pos = offset+subLen;
            continue;
        }
        if (wd[offset+subLen]=='\0' || wd[offset+subLen]==sep) {
            /* matched with those right structure like 'home/something' or 'system/home'
             * then set the correct characters to '\0' */
            for (int i=0; i<subLen; i++)
                wd[offset+i] = '\0';
            wd[0] = '~';
            /* move the characters behind homeWd to front */
            offset += subLen;
            strcpy(wd+1, wd+offset);
            for (offset=mainLen-1; wd[offset]!='\0'; offset--);
            memset(wd+offset, '\0', sizeof(char)*(mainLen-offset));
            /* if substr is too far back in main, some characters will be left behind
             * make sure there are no unwanted characters in the string */
            exitFlag = TRUE; 
        }
        else if (offset == -1)
            exitFlag = TRUE;
    } while (exitFlag == FALSE);
}
