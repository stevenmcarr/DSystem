/* $Id: ColumnEditor.h,v 1.2 1997/03/11 14:32:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/ColumnEditor.h					*/
/*									*/
/*	ColumnEditor -- Abstract class for column-oriented editors	*/
/*	Last edited: October 13, 1993 at 12:34 pm			*/
/*									*/
/************************************************************************/




#ifndef ColumnEditor_h
#define ColumnEditor_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/LineEditor.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/***********************/
/* ColumnEditor class */
/***********************/




struct ColumnEditor_Repr_struct;




class ColumnEditor: public LineEditor
{
public:

  ColumnEditor_Repr_struct * ColumnEditor_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */

  META_DEF(ColumnEditor)
				ColumnEditor(Context context, DB_FP * session_fd);
  virtual			~ColumnEditor(void);

/* access to contents */
  virtual void			SetSortColumn(int colNum);
  virtual void			GetSortColumn(int& colNum);

};




#endif /* __cplusplus */

#endif /* not ColumnEditor_h */
