/* $Id: utility.h,v 1.1 1997/03/11 14:29:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// This may look like c code but it is really -*- C++ -*-


/* If no bits set, returns -1. Otherwise returns max/min bit set in 
   numbering scheme where msb is 31 and lsb 0.  
*/

int Max_Set_Bit32(unsigned long i);
int Min_Set_Bit32(unsigned long i);

char * dupstr(const char * c);

void SetString(char * & variable, const char * string);




