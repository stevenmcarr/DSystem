/* $Id: SubprogDecoration.h,v 1.2 1997/03/11 14:30:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/SubprogDecoration.h					*/
/*									*/
/*	SubprogDecoration -- Containing subprog name in left margin	*/
/*	Last edited: October 14, 1993 at 3:43 pm			*/
/*									*/
/************************************************************************/




#ifndef SubprogDecoration_h
#define SubprogDecoration_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/MarginDecoration.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/******************************/
/* SubprogDecoration class */
/******************************/




struct SubprogDecoration_Repr_struct;
class  DedDocument;




class SubprogDecoration: public MarginDecoration
{
public:

  SubprogDecoration_Repr_struct * SubprogDecoration_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static SubprogDecoration *	Create(DedDocument * doc);

/* initialization */
  META_DEF(SubprogDecoration)
				SubprogDecoration(void);
  virtual void			Init(DedDocument * doc);
  virtual void			Destroy(void);
				~SubprogDecoration(void);

/* margin text */
  virtual int			Width(void);
  virtual void			GetMarginText(int c_linenum,
                                              TextChar * textstring_tc_ptr,
                                              ColorPair * textdata_chars);

};




#endif /* __cplusplus */

#endif /* not SubprogDecoration_h */
