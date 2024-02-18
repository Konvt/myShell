#include <stdio.h> // stderr

#include "error_handle.h"
#include "font_style.h"

void throw_error(const char* where, const char* what)
{
  const char* error_fmt = STYLIZE("%s:", FNT_YELLOW) STYLIZE(" %s\n", FNT_RED);
  fprintf(stderr, error_fmt, where, what);
}
