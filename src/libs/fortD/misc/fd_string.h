/* $Id: fd_string.h,v 1.4 1997/06/24 17:40:05 carr Exp $ */
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

#endif 
