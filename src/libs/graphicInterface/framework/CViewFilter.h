/* $Id: CViewFilter.h,v 1.4 1997/03/11 14:32:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/CViewFilter.h						*/
/*									*/
/*	CViewFilter -- Determines how a CTextView displays lines	*/
/*	Last edited: October 13, 1993 at 12:47 pm			*/
/*									*/
/************************************************************************/




#ifndef CViewFilter_h
#define CViewFilter_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/DBObject.h>
#include <libs/graphicInterface/framework/Text.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/******************/
/* Elision styles */
/******************/




/* must agree with "ned_cp/ViewFilter.h" */

#define ViewFilter_ELISION_NONE		0
#define ViewFilter_ELISION_ELLIPSIS	1
#define ViewFilter_ELISION_DIVLINE	2




/*********************/
/* CViewFilter class */
/*********************/




struct CViewFilter_Repr_struct;
class  CTextView;




class CViewFilter: public DBObject
{
  friend CTextView;


public:

  CViewFilter_Repr_struct * CViewFilter_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(CViewFilter)
				CViewFilter(void);
				CViewFilter(Context context, DB_FP * session_fp);
  virtual			~CViewFilter(void);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* access to filter specs */
  virtual void			SetElision(int elision);
  virtual void			GetElision(int &elision);

/* coordinate conversion */
  virtual void			GetDocSize(Point &size);
  virtual Boolean		ContentsLineElided(int c_lineNum);
  virtual int			ContentsToViewLinenum(int c_lineNum);
  virtual int			ViewToContentsLinenum(int v_lineNum);


protected:

/* database */
  virtual void			isnew(Context context);
  virtual void			read(DB_FP * fp, DB_FP * session_fp);


public:		/* pretend this is 'protected' */

/* filtering */
  virtual Boolean		filterLine(Boolean countOnly,
                                           int line,
                                           int &subline,
                                           TextString &text,
                                           TextData &data);

private:

/* interface to Ned ViewFilter */
  virtual Generic		getNedViewFilter(void);

};




#endif /* __cplusplus */

#endif /* not CViewFilter_h */
