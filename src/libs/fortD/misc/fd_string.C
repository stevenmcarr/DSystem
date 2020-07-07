/* $Id: fd_string.C,v 1.4 1997/03/11 14:28:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id $ */
#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>

using namespace std;
#include <libs/fortD/misc/fd_string.h>

//------------------------------------------------------------------
//------------------------------------------------------------------
FD_String::FD_String()
{
  size = 0;
  string = 0;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
FD_String::~FD_String()
{
  if (size == 0)
    cout << "String not allocated\n";
  else if (size < 0)
    cout << "String freed twice\n";
  else 
  {
   free(string);
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FD_String::Append(char* st)
{
 char* str;
 int sze;

// if a string exists then append the new string

 if (size) { 
  sze = size + strlen(st) + 1;
  str = string;
  string =  (char*)malloc(sze);
  strcpy(string, str);
  strcpy(&string[size-1], st);
  delete str;
  size = sze;
 }

 else {
 size = strlen(st) + 1;
 string = (char*)malloc(size);
 strcpy(string, st);
 }
};

void FD_String::Print()
{
 cout << string << '\n' ;
};
