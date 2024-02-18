#ifndef __SEL_IO_UTIL__
  #define __SEL_IO_UTIL__

#include <stdio.h> // FILE

#include "constant_def.h"

void input_str(FILE* infile, char* dest, int char_limit, const char* prompt);

#endif
