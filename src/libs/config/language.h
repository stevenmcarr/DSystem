/* $Id: language.h,v 1.1 1997/03/11 14:27:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef language_h
#define language_h

/* external declarations for Rn_language from config.c */

extern char *Rn_language;

typedef enum 
{
   Lang_F77, 
   Lang_F77D, 
   Lang_F77SMP, 
   Lang_F90D
} LanguageChoice;

#endif
