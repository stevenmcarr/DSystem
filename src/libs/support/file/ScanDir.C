/* $Id: ScanDir.C,v 1.7 2001/09/17 01:27:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*******************************************************************************
 *                                                                             *
 *  File:    scandir.C                                                         *
 *  Author:  Kevin Cureton modifications based on net version                  *
 *                                                                             *
 *  Usage:                                                                     *
 *    scandir(dir, area, select, compare)                                      *
 *       dir - directory to scan                                               *
 *       flist - pointer to pointer to an array of pointer's to struct dirent  *
 *       selectFunct - pointer to function returning an int                    *
 *       compareFunct - pointer to function returning an int                   *
 *                                                                             *
 *******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <libs/support/file/ScanDir.h>

static const int FAIL = -1;
static const int OVERFLOW = 10;
static const int MAX_NAME_LEN = 256;  // No one header file has a definition
                                      // for this among the various OS's.

typedef FUNCTION_POINTER (int, QSortCompareFunctPtr, (const void*, const void*));

int
scandir(char* dir, 
        struct dirent*** flist, 
        SelectFunctPtr selectFunct, 
        CompareFunctPtr compareFunct)
{
  DIR* fp;
  struct dirent*  dirptr;
  struct dirent** temp;
  uint count = 0;
  uint direntSize = sizeof(struct dirent);
  uint direntPtrSize = sizeof(struct dirent*);

  if ((fp = opendir(dir)) == (DIR*)NULL)
  {   // could not open the dir
    return FAIL;
  }

    // count the number of entries in the directory
  for (count = 0, dirptr = readdir(fp); dirptr != (struct dirent*)NULL; dirptr = readdir(fp)) 
  {
    if (selectFunct(dirptr)) count++;
  }

  if (count == 0)
  { 
    closedir(fp);
    return 0;
  }

     // allocate a contiguous space of memory for pointers to the entries
     // and allow for new files that might be created
  temp = (struct dirent**) new char[(count + OVERFLOW) *  direntPtrSize]; 
  if (temp == (struct dirent**)NULL)
  {
    closedir(fp);
    return FAIL;
  }

  for (int i = 0; i < count + OVERFLOW; i++) temp[i] = (struct dirent*)NULL;
  
    // rewind the directory
  rewinddir(fp);

    // put the pointers to all active elements in the newly allocated array
  for(count = 0, dirptr = readdir(fp); dirptr != (struct dirent*)NULL; dirptr = readdir(fp))
  {
    if (selectFunct(dirptr))
    {
      temp[count] = (struct dirent*)new char [direntSize + MAX_NAME_LEN];
      if (temp[count] == (struct dirent*)NULL) 
      {
        closedir(fp);
        for (int i = 0; i < count; i++) delete temp[i];
        delete temp;
        return FAIL;
      }
      else
      {
        memset ((void*)temp[count], 0, direntSize);
      }

#     ifdef _AIX
        temp[count]->d_ino = dirptr->d_ino;             // put it in
        temp[count]->d_offset = dirptr->d_offset;       // put it in
        temp[count]->d_reclen = dirptr->d_reclen;       // put it in
        temp[count]->d_namlen = dirptr->d_namlen;       // put it in
        strcpy (temp[count]->d_name, dirptr->d_name);   // put it in
#     else
        temp[count]->d_ino = dirptr->d_ino;             // put it in
        temp[count]->d_off = dirptr->d_off;             // put it in
        temp[count]->d_reclen = dirptr->d_reclen;       // put it in
        strcpy (temp[count]->d_name, dirptr->d_name);   // put it in
#     endif

      count++;                                          // tell me
    }
  }

    // now sort the entries
  qsort((char*)temp, count, sizeof(struct dirent*), (QSortCompareFunctPtr)compareFunct);

  *flist = temp;

  closedir(fp);

  return (int)count;
}

int alphasort(dirent** d1, dirent** d2)
{
  return strcmp((*d1)->d_name, (*d2)->d_name);
}
