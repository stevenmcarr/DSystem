/* $Id: DedDependenceArrow.h,v 1.2 1997/03/11 14:30:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/src/DedDependenceArrow.h					*/
/*									*/
/*	DedDependenceArrow -- Ded custom-colored dependence arrow	*/
/*	Last edited: November 6, 1993 at 2:09 pm			*/
/*									*/
/************************************************************************/




#ifndef DedDependenceArrow_h
#define DedDependenceArrow_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/DependenceArrow.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus





/****************************/
/* DedDependenceArrow class */
/****************************/




struct DedDependenceArrow_Repr_struct;
class  DedDocument;




class DedDependenceArrow: public DependenceArrow
{
public:

  DedDependenceArrow_Repr_struct * DedDependenceArrow_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static DedDependenceArrow *	Create(Dependence dep, DedDocument * doc);

/* initialization */
  META_DEF(DedDependenceArrow)
				DedDependenceArrow(void);
  virtual void			Init(Dependence dep, DedDocument * doc);
  virtual void			Destroy(void);
  virtual			~DedDependenceArrow(void);


protected:
  
/* appearance */
  virtual void			getShaft(int &width, LineStyle * &style, Color &color);

};




#endif /* __cplusplus */

#endif /* not DedDependenceArrow_h */
