/* $Id: CFortEditor.h,v 1.2 1997/03/11 14:32:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/CFortEditor.h						*/
/*									*/
/*	CFortEditor -- Fortran source code editor			*/
/*	Last edited: October 13, 1993 at 12:24 pm			*/
/*									*/
/************************************************************************/




#ifndef CFortEditor_h
#define CFortEditor_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/LineEditor.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/fortTree/FortTree.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*********************/
/* CFortEditor class */
/*********************/




struct CFortEditor_Repr_struct;
class  UserFilterDef;
class  FilterDefSet;
class  Editor;




class CFortEditor: public LineEditor
{
public:

  CFortEditor_Repr_struct * CFortEditor_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(CFortEditor)
				CFortEditor(Context context, DB_FP * session_fd);
  virtual			~CFortEditor(void);

/* database */
  virtual void			Open(Context context,
		                     Context mod_in_pgm_context,
		                     Context pgm_context,
		                     DB_FP * fd,
                                     DBObject * contents,
		                     FortTextTree ftt,
		                     FortTree ft);
  virtual void			Close(void);
  virtual void			Save(Context context, DB_FP * fd);
  virtual void			GetContentsAttribute(char * &contents_attr);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* viewing */
  virtual View *		OpenView(Context context, DB_FP * session_fd);

/* filtering */
  virtual UserFilterDef *	OpenFilterDef(Context context,
				              Context mod_in_pgm_context,
				              Context pgm_context,
				              DB_FP * session_fd);
  virtual void			AddStandardFilterDefs(FilterDefSet * defs);

/* input handling */
  virtual Boolean		MenuChoice(Generic cmd);
  virtual void			Keystroke(KbChar kb);

/* access to contents */
  virtual int			NumLines(void);
  virtual int			MaxLineWidth(void);
  virtual void			GetLine(int k, TextString &ts, TextData &td);
  virtual void			GetLineInfo(int k, FortTreeNode &node, int &bracket);
  virtual void			NodeToText(FortTreeNode node,
                                           int &line1,
                                           int &char1,
                                           int &line2,
                                           int &char2);
  virtual void			TextToNode(int line1,
                                           int char1,
                                           int line2,
                                           int char2,
                                           FortTreeNode &node);

/* access to selection */
  virtual void			GetSelection(Selection * &sel);
  virtual void			GetSelection(int &line1, int &char1, int &line2, int &char2);
  virtual void			SetSelection(Selection * sel);
  virtual void			SetSelection(int line1, int char1, int line2, int char2);
  virtual void			GetSelectedData(Generic &data);
  virtual void			SetSelectedData(Generic data);

/* editing */
  virtual Scrap *		Extract(Selection * sel);
  virtual void			Replace(Selection * sel, Scrap * scrap);

/* errors */
  virtual Boolean		CheckLineData(int k);
  virtual Boolean		CheckData(void);

/* navigation */
  virtual void			NavigateTo(FortTreeNode target);

/* access to Fortran contents */
  virtual FortTree		GetFortTree(void);
  virtual FortTextTree		GetFortTextTree(void);
  virtual void			SetCurrentLoop(FortTreeNode loop);


public:

/* trap door for CPed */
  virtual Generic		getFortEditor(void);
};




#endif /* __cplusplus */

#endif /* not CFortEditor_h */
