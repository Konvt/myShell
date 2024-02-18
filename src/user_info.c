#include <time.h>   // time
#include <stdlib.h> // malloc, memset, free
#include <string.h> // strcpy, strpbrk
#include <dirent.h> // DIR, opendir, closedir

#include "io_util.h"
#include "error_handle.h"
#include "user_info.h"
#include "functional_func.h"

#include "constant_def.h"
#include "font_style.h"

const uint32_t uid_len = 8;
const uint32_t name_limit = 32;

usr_info* create_usr(usr_info* const this)
{
  this->uid = 0;
  this->name = (char*)malloc((name_limit + 1) * sizeof(char));
  if (this->name == NULL)
    return NULL;

  this->destructor = &release_usr;
  this->set_uid = &generate_uid;

  return this;
}

void release_usr(usr_info* const this)
{
  release_ptr(this->name);
}

usr_info* generate_uid(usr_info* const this)
{
  this->uid = time(NULL);
  return this;
}

usr_info* make_usr_info(usr_info* const this)
{
  usr_info* result = create_usr(this);
  if (result == NULL)
    return NULL;
  this->set_uid(this);

  char* blank_char = NULL;
  do {
    input_str(stdin, this->name, name_limit, STYLIZE("Login as", FNT_CYAN FNT_BOLD_TEXT) ": ");
    blank_char = strpbrk(this->name, " \t\n\v\f\r");
    if (blank_char != NULL)
      throw_error("login", "whitespace characters are not allowed");
  } while ( blank_char != NULL );
  
  DIR *dir = opendir(this->name);
  if (dir == NULL && make_dir(&this->name, 1) == failed) {
    throw_error("login", "create home dir failure");
    return NULL;
  }
  closedir(dir);
  change_wd(this->name);

  return this;
}
