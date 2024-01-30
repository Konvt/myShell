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
  if (str == NULL || terminus <= 0)
    return NULL; // an error offset

  size_t index;
  /* unlimit string interception range by address */
  for (index = terminus - 1; (str + index) >= str; index--) {
    if (isspace(str[index]))
      str[index] = BLANK;
    else if (str[index] != BLANK)
      break;
  }
  for (index = 0; (str + index) < (str + terminus); index++) {
    if (isspace(str[index]))
      str[index] = BLANK;
    else break;
  }

  return str + index;
}

void GetInput(FILE* src, char* acceptInput, int inputLimit, const char* prompt)
{
  if (acceptInput == NULL || inputLimit == 0) return;
  char *hadTrimmed = NULL;

  fflush(src);
  acceptInput[0] = BLANK;
  do {
    if (prompt != NULL && acceptInput[0] == BLANK)
      fputs(prompt, stdout);
    fgets(acceptInput, inputLimit, src);
    int len = strlen(acceptInput);
    hadTrimmed = StrTrim(acceptInput, len + 1);
  } while (hadTrimmed[0] == BLANK);

  /* if there are spaces at the beginning
   * some non-null characters will stay in the middle of the string
   * thus we need move them to front */
  strcpy(acceptInput, hadTrimmed);
  /* make sure there are not any unwanted characters behind */
  for (int i = 0; ; i++) {
    if (acceptInput[i] == BLANK) {
      memset(acceptInput + i + 1, BLANK, inputLimit - i - 1);
      break;
    }
  }
}

int MatchSubstr(const char* mainstr, const char* substr, int pos)
{
  const char *ptr = strstr(mainstr + pos, substr);
  return ptr == NULL ? -1 : (int)(ptr - mainstr);
}

void AdjustDir(char* wd, const char* homeWd)
{ /* this function will set the string like 'system/home/name' to be '~/name' */
  const char sep = '/';
  int mainLen = strlen(wd), subLen = strlen(homeWd);

  int pos = 0, exitFlag = FALSE;
  do {
    int offset = MatchSubstr(wd, homeWd, pos);
    if (offset == -1) break;
    else if (offset - 1 >= 0 && wd[offset - 1] != sep) {
      /* when we match something like 'byhome/'
       * it's an error match, rematches with a new pos */
      pos = offset + subLen;
      continue;
    }
    if (wd[offset + subLen] == BLANK || wd[offset + subLen] == sep) {
      /* matched with those right structure like 'home/something' or 'system/home'
       * then set the correct characters to BLANK */
      for (int i = 0; i < subLen; i++)
        wd[offset + i] = BLANK;
      wd[0] = '~';
      /* move the characters behind homeWd to front */
      offset += subLen;
      strcpy(wd + 1, wd + offset);
      for (offset = mainLen - 1; wd[offset] != BLANK; offset--);
      memset(wd + offset, BLANK, sizeof(char) * (mainLen - offset));
      /* if substr is too far back in main, some characters will be left behind
       * make sure there are no unwanted characters in the string */
      exitFlag = TRUE;
    }
  } while (exitFlag == FALSE);
}
