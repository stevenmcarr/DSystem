/* $Id: LoopIconDecoration.h,v 1.2 1997/03/11 14:32:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/LoopIconDecoration.h					*/
/*									*/
/*	LoopIconDecoration -- Loop icons in margin, headers colorized	*/
/*	Last edited: October 13, 1993 at 6:28 pm			*/
/*									*/
/************************************************************************/




#ifndef LoopIconDecoration_h
#define LoopIconDecoration_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/MarginDecoration.h>

#include <libs/frontEnd/fortTree/FortTree.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/****************************/
/* LoopIconDecoration class */
/****************************/




struct LoopIconDecoration_Repr_struct;
class  CTextView;
class  LineNumDecoration;




class LoopIconDecoration: public MarginDecoration
{
public:
  LoopIconDecoration_Repr_struct * LoopIconDecoration_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static LoopIconDecoration *	Create(LineNumDecoration * linenumDec);

/* initialization */
  META_DEF(LoopIconDecoration)
				LoopIconDecoration(void);
  virtual void			Init(LineNumDecoration * linenumDec);
				~LoopIconDecoration(void);

/* options */
  virtual void			SetOptions(Color foreColor, Boolean colorizeWholeLoop);
  virtual void			GetOptions(Color &foreColor, Boolean &colorizeWholeLoop);

/* drawing */
  virtual void			ColorizeLine(int c_linenum,
                                             int marginWidth,
                                             TextString &text,
                                             TextData &data);
  virtual Rectangle		BBox(void);

/* text */
  virtual int			Width(void);
  virtual void			GetMarginText(int c_linenum,
                                              TextChar * textstring_tc_ptr,
                                              ColorPair * textdata_chars);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* input */
  virtual void			Clicked(int c_linenum, int char1);

/* current loop */
  virtual void			GetCurrentLoop(FortTreeNode &node, Rectangle &bbox);


protected:
  
/* display calculation*/
  virtual void			calcCurLoop(FortTreeNode loopNode);
  virtual Boolean		isLoopHeader(int c_linenum, FortTreeNode &node, int &bracket);

};




#endif /* __cplusplus */

#endif /* not LoopIconDecoration_h */
