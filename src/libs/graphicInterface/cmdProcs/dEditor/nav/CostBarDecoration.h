/* $Id: CostBarDecoration.h,v 1.2 1997/03/11 14:30:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/nav/CostBarDecoration.h					*/
/*									*/
/*	CostBarDecoration -- Barchart of comm/comp costs in left margin	*/
/*	Last edited: October 14, 1993 at 1:20 pm			*/
/*									*/
/************************************************************************/




#ifndef CostBarDecoration_h
#define CostBarDecoration_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/MarginDecoration.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/***************************/
/* CostBarDecoration class */
/***************************/




struct CostBarDecoration_Repr_struct;
class  DedDocument;




class CostBarDecoration: public MarginDecoration
{
public:

  CostBarDecoration_Repr_struct * CostBarDecoration_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static CostBarDecoration *	Create(DedDocument * doc);

/* initialization */
  META_DEF(CostBarDecoration)
				CostBarDecoration(void);
  virtual void			Init(DedDocument * doc);
  virtual void			Destroy(void);
				~CostBarDecoration(void);

/* margin text */
  virtual int			Width(void);
  virtual void			GetMarginText(int c_linenum,
                                              TextChar * textstring_tc_ptr,
                                              ColorPair * textdata_chars);

};




#endif /* __cplusplus */

#endif /* not CostBarDecoration_h */
