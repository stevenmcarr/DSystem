/* $Id: fd_string.h,v 1.3 1997/03/11 14:28:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id $*/
#ifndef _fd_string_
#define _fd_string_

#include <assert.h>
#include <stdlib.h>

class FD_String
{
  private:
    int size;

  public:
    char *string;

 public:
   FD_String();
  ~FD_String();

   void Append(char*);
   void Print();
};

#endif _fd_string_
