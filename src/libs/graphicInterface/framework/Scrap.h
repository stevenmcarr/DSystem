/* $Id: Scrap.h,v 1.4 1997/03/11 14:32:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Scrap.h						*/
/*									*/
/*	Scrap -- Abstract class for edit scraps				*/
/*	Last edited: October 13, 1993 at 10:13 pm			*/
/*									*/
/************************************************************************/




#ifndef Scrap_h
#define Scrap_h


#include <libs/graphicInterface/framework/framework.h>



/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/****************/
/* Scrap class */
/****************/




struct Scrap_Repr_struct;




class Scrap: public Object
{
public:

  Scrap_Repr_struct * Scrap_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(Scrap)
				Scrap(void);
  virtual			~Scrap(void);

/* ... */

};




#endif /* __cplusplus */

#endif /* not Scrap_h */
