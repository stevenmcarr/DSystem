/* $Id: CClipboard.h,v 1.4 1997/03/11 14:32:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/CClipboard.h						*/
/*									*/
/*	CClipboard -- Class of the unique global clipboard		*/
/*	Last edited: October 13, 1993 at 12:03 am			*/
/*									*/
/************************************************************************/




#ifndef CClipboard_h
#define CClipboard_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/Scrap.h>





/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/********************/
/* CClipboard class */
/********************/




struct CClipboard_Repr_struct;




class CClipboard: public Object
{
public:

  CClipboard_Repr_struct * CClipboard_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(CClipboard)
				CClipboard(void);
  virtual			~CClipboard(void);

/* access to scrap */
  virtual Scrap *		GetScrap(void);
  virtual void			SetScrap(Scrap * scrap);

};




/************************/
/* The global clipboard */
/************************/




extern CClipboard * theClipboard;






#endif /* __cplusplus */

#endif /* not CClipboard_h */
