/* $Id: utility.C,v 1.1 1997/03/11 14:29:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// This may look like c code but it is really -*- C++ -*-

/* Divide and conquer classic:
   Keep on narrowing zone in which max bit could lie.
   1111111100000000 0xff00
   1111000011110000 0xf0f0
   1100110011001100 0xcccc
   1010101010101010 0xaaaa

   returns -1 if no bits set

*/
#include <string.h>

int Max_Set_Bit32(unsigned long i) {
  int out;
  if (i == 0) 
    out = -1;
  else {
    out = 0;
    if (i & 0xffff0000) out +=16;
    if (i & 0xff00ff00) out +=8;
    if (i & 0xf0f0f0f0) out +=4;
    if (i & 0xcccccccc) out +=2;
    if (i & 0xaaaaaaaa) out +=1;
  }
  return out;
}

int Min_Set_Bit32(unsigned long i) {
  int out;
  if (i == 0)
    out = -1;
  else {
    out = 32;
    if (i & 0x0000ffff) out -=16;
    if (i & 0x00ff00ff) out -=8;
    if (i & 0x0f0f0f0f) out -=4;
    if (i & 0x33333333) out -=2;
    if (i & 0x55555555) out -=1;
  }
  return out;
}

char * dupstr(const char * c) {
  int l = strlen(c);
  char * s = new char[l+1];
  strcpy(s,c);
  return s;
}

void SetString(char * & variable, const char * string) {
  if (variable !=0) {
    delete[] variable;
  }
  variable = dupstr(string);
}
