/* $Id: DependenceArrow.h,v 1.2 1997/03/11 14:32:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/DependenceArrow.h					*/
/*									*/
/*	DependenceArrow -- Arrow in a FortView depicting a dependence	*/
/*	Last edited: October 13, 1993 at 5:45 pm			*/
/*									*/
/************************************************************************/




#ifndef DependenceArrow_h
#define DependenceArrow_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/FortArrow.h>
#include <libs/graphicInterface/framework/Dependence.h>
#include <libs/graphicInterface/framework/Text.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*************************/
/* DependenceArrow class */
/*************************/




struct DependenceArrow_Repr_struct;




class DependenceArrow: public FortArrow
{
public:

  DependenceArrow_Repr_struct * DependenceArrow_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static DependenceArrow *	Create(Dependence dep, ColorPair colors);

/* initialization */
  META_DEF(DependenceArrow)
				DependenceArrow(void);
  virtual void			Init(Dependence dep, ColorPair colors);
  virtual void			Destroy(void);
  virtual			~DependenceArrow(void);
    
};




#endif /* __cplusplus */

#endif /* not DependenceArrow_h */
