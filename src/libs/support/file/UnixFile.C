/* $Id: UnixFile.C,v 1.1 1997/06/25 15:14:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/support/misc/general.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/file/UnixFile.h>

Boolean file_access(const char* filename, int mode)
{
  return BOOL( access( filename, mode) != -1 );
}

Boolean file_ok_to_write(const char* filename)
{
  int fd;
  int i;
  char *temp;
  struct stat buf;
  char working_dir[MAXPATHLEN];

  if (filename[0] == '/')
    temp = ssave(filename);
  else
  {
    if (getcwd(working_dir, MAXPATHLEN) == 0)
      return false;
    else
      temp = nssave(3, working_dir, "/", filename);
  }

  /* if it is a directory we can't plop a file on top */
  i = stat (temp, &buf);

  /* Can't use "buf" if stat fails (bbc & mwh 12/16/90) */
  if (i != -1)
  {/* added extra cases of files that can't be written (bbc 12/16/90) */
    if (S_ISDIR(buf.st_mode)  ||
        S_ISFIFO(buf.st_mode) ||
        S_ISCHR(buf.st_mode)  ||
        S_ISBLK(buf.st_mode)  ||
        S_ISSOCK(buf.st_mode))
      {
	sfree(temp);
	return false;
      }
  }

  fd = access( temp, W_OK);

  /* can't open, it may not exist  */
  if (fd < 0)
  {
    /* Write permission in containing directory? temp definitely 
       contains a '/' */
    for (i = strlen(temp) - 1; temp[i] != '/'; i--);
    temp[i] = '\0';
    fd = access (temp, W_OK);
    if (fd < 0)
    {
      sfree(temp);
      return false;	
    }
  }

  sfree (temp);
  return true;
}

/*
 * Touch the unix file -- create the file if it exists. Returns true if
 * the file exists or we were able to create it.
 */
Boolean file_touch(const char* filename)
{
  FILE *fp;

  fp = fopen(filename, "r");
  /* assume that the file does not exist, if we can't open it for reading */
  if (fp == NULL)
  {
    /* create it */
    fp = fopen(filename, "w");

    /* problems ... */
    if (fp == NULL)
      return false;
  }

  fclose(fp);
  return true;
}

Boolean file_is_empty(const char* filename)
{
  struct stat buf;
  return BOOL((stat (filename, &buf) != -1) && (buf.st_size == 0));
}

