/* $Id: CTextView.h,v 1.4 1997/03/11 14:32:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/CTextView.h						*/
/*									*/
/*	CTextView -- Wrapper for Ned's TextView.c			*/
/*	Last edited: November 11, 1993 at 12:12 am			*/
/*									*/
/************************************************************************/




#ifndef CTextView_h
#define CTextView_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/View.h>
#include <libs/graphicInterface/framework/Text.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>

#include <libs/graphicInterface/cmdProcs/newEditor/TextView.h>
#include <libs/support/arrays/FlexibleArray.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*******************/
/* CTextView class */
/*******************/




struct CTextView_Repr_struct;
class  Editor;
class  Decoration;
class  MarginDecoration;




class CTextView: public View
{
public:

  CTextView_Repr_struct * CTextView_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(CTextView)
				CTextView(Context context,
                                          DB_FP * session_fp,
                                          Editor * editor,
                                          Point initScrollPos,
                                          int font);
  virtual			~CTextView(void);
  virtual void			CustomizeRepainting(tv_CustomRepaintFunc repaintProc);

/* database */
  virtual void			Save(Context context, DB_FP * session_fp);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* window layout */
  virtual Generic		GetTiling(Boolean init, Point size);
  virtual void			InitPanes(void);
  virtual Point			TextSize(void);
  virtual void			SetScrollBars(Generic hscroll, Generic vscroll);

/* scrolling */
  virtual Point			GetScroll(void);
  virtual void			SetScroll(Point scrollPos);
  virtual void			ScrollBy(Point delta);
  virtual void			EnsureVisible(Point pt);
  virtual void			EnsureSelVisible(void);

/* filtering */
  virtual void			SetFilter(CViewFilter * filter);

/* mapping linenums */
  virtual Boolean		ContentsLineElided(int c_lineNum);
  virtual int			ContentsToViewLinenum(int c_lineNum);
  virtual int			ContentsToScreenLinenum(int c_lineNum);
  virtual int			ViewToContentsLinenum(int v_lineNum);
  virtual int			ViewToScreenLinenum(int v_lineNum);
  virtual int			ScreenToContentsLinenum(int s_lineNum);
  virtual int			ScreenToViewLinenum(int s_lineNum);

/* mapping points */
  virtual Point			ContentsToViewPoint(Point c_pt);
  virtual Point			ContentsToScreenPoint(Point c_pt);
  virtual Point			ViewToContentsPoint(Point v_pt);
  virtual Point			ViewToScreenPoint(Point v_pt);
  virtual Point			ScreenToContentsPoint(Point s_pt);
  virtual Point			ScreenToViewPoint(Point s_pt);

/* mapping rectangles */
  virtual Rectangle		ContentsToViewRect(Rectangle c_rect);
  virtual Rectangle		ContentsToScreenRect(Rectangle c_rect);
  virtual Rectangle		ViewToContentsRect(Rectangle v_rect);
  virtual Rectangle		ViewToScreenRect(Rectangle v_rect);
  virtual Rectangle		ScreenToContentsRect(Rectangle s_rect);
  virtual Rectangle		ScreenToViewRect(Rectangle s_rect);

/* mapping between chars and pixels */
  virtual Rectangle		CharToPixelRect(Rectangle ch_rect);
  virtual Rectangle		PixelToCharRect(Rectangle px_rect);

/* drawing */
  virtual Point			CharacterSize(void);
  virtual void			BeginClip(Rectangle clip);
  virtual void			EndClip(void);
  virtual Rectangle		GetClip(void);
  virtual void			SetDefaultColors(Color foreColor,
                                                 Color backColor,
                                                 Color borderColor,
                                                 Boolean useParentColors,
                                                 Boolean invertColors,
                                                 Boolean updateScreen);
  virtual void			SetDrawingStyle(int lineWidth,
                                                LineStyle * lineStyle,
                                                Color fg,
                                                Color bg);
  virtual void			GetDrawingStyle(int &lineWidth,
                                                LineStyle * &lineStyle,
                                                Color &fg,
                                                Color &bg);
  virtual void			DrawLine(Point p1, Point p2);
  virtual void			DrawRect(Rectangle r);
  virtual void			DrawArc(Rectangle box, double angle, double extent);
  virtual void			DrawPolygon(Point start, int numPts, Point * points);

/* decorating */
  virtual void			AddDecoration(Decoration * d);
  virtual void			RemoveDecoration(Decoration * d);
  virtual void			RemoveAllDecorations(void);
  virtual void			AddMarginDecoration(MarginDecoration * md);
  virtual void			RemoveMarginDecoration(MarginDecoration * md);
  virtual void			RemoveAllMarginDecorations(void);

/* input handling */
  virtual Boolean		SelectionEvent(Generic generator, Point info);
  virtual void			SetSelectionOptions(Boolean oneLine, Boolean notElidedLine);


public:		/* pretend this is 'private' ! */

/* access to contents */
  virtual int			numLines(void);
  virtual int			maxLineWidth(void);
  virtual void			getLine(int k, TextString &ts, TextData &td);

/* access to selection */
  virtual void			getSelection(int &line1, int &char1, int &line2, int &char2);
  virtual void			setSelection(int line1, int char1, int line2, int char2);

/* filtering */
  virtual CViewFilter *		makeDefaultFilter(void);

/* decorating */
  virtual void			addDecoration(Flex * decs, Decoration * d, Boolean first);
  virtual void			removeDecoration(Flex * decs, Decoration * d);
  virtual void			layoutMarginDecorations(void);
  virtual void			update(void);
  virtual void			decorateMargins(int c_linenum, TextString &text, TextData &data);
  virtual void			colorizeLine(int c_linenum, TextString &text, TextData &data);
  virtual void			drawDecorations(int pane_line1, int pane_line2);
  virtual void			marginDecorationClicked(int c_linenum, int char1);

};




#endif /* __cplusplus */

#endif /* not CTextView_h */
