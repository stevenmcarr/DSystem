/* $Id: LineEditor.h,v 1.4 1997/03/11 14:32:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/LineEditor.h						*/
/*									*/
/*	LineEditor -- Editor for things like line-structured text	*/
/*	Last edited: October 13, 1993 at 6:17 pm			*/
/*									*/
/************************************************************************/




#ifndef LineEditor_h
#define LineEditor_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/DBObject.h>
#include <libs/graphicInterface/framework/Text.h>
#include <libs/graphicInterface/framework/View.h>
#include <libs/graphicInterface/framework/Selection.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/***********************/
/* Change Notification */
/***********************/




typedef struct
  {
    int	kind;			/* kind of change */
    Boolean	autoScroll;	/* whether UI implies autoscroll for this change */
    Generic	data;
    int	first;			/* first line number affected */
    int	last;			/* last  line number affected */
    int	delta;			/* number of lines added or deleted */

  } LineEditorChange;






/********************/
/* LineEditor class */
/********************/




struct LineEditor_Repr_struct;
class  View;




class LineEditor: public Editor
{
public:

  LineEditor_Repr_struct * LineEditor_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(LineEditor)
				LineEditor(Context context, DB_FP * session_fp);
  virtual			~LineEditor(void);

/* access to contents */
  virtual int			NumLines(void);
  virtual int			MaxLineWidth(void);
  virtual void			GetLine(int k, TextString &ts, TextData &td);
  virtual void			SetLine(int k, TextString line);
  virtual void			InsertLine(int k, TextString line);
  virtual void			DeleteLine(int k);

/* selection */
  virtual void			GetSelection(Selection * &sel);
  virtual void			GetSelection(int &line1, int &char1, int &line2, int &char2);
  virtual void			SetSelection(Selection * sel);
  virtual void			SetSelection(int line1, int char1, int line2, int char2);
  virtual void			GetSelectedData(Generic &data);
  virtual void			SetSelectedData(Generic data);
  virtual void			SetSelectionNone(void);
  virtual Boolean		HasSelection(void);
  virtual Boolean		IsSelection(int line1, int char1, int line2, int char2);

/* editing */
  virtual Scrap *		Extract(Selection * sel);
  virtual void			Replace(Selection * sel, Scrap * scrap);

/* errors */
  virtual Boolean		CheckLineData(int k);
  virtual Boolean		CheckData(void);
  virtual void			SetShowErrors(Boolean show);

/* change notification */
  virtual void			ChangedEverything(void);
  virtual void			clearDeferredChanges(void);
  virtual void			addDeferredChange(int kind, void * change);
  virtual void			getDeferredChanges(int &kind, void * &change);


protected:

/* initialization */
  virtual void			setContents(DBObject * contents);

};




#endif /* __cplusplus */

#endif /* not LineEditor_h */
