/* $Id: DepEditor.h,v 1.2 1997/03/11 14:30:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/dep/DepEditor.h						*/
/*									*/
/*	DepEditor -- Ded Dependences sub-editor				*/
/*	Last edited: November 6, 1993 at 10:25 pm			*/
/*									*/
/************************************************************************/




#ifndef DepEditor_h
#define DepEditor_h


#include <libs/graphicInterface/framework/framework.h>

#include <libs/graphicInterface/framework/ColumnEditor.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*******************/
/* DepEditor class */
/*******************/




struct DepEditor_Repr_struct;
class  UserFilter;
class  UserFilterDef;
class  FilterDefSet;
class  DedEditor;




class DepEditor: public ColumnEditor
{
public:

  DepEditor_Repr_struct * DepEditor_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(DepEditor)
				DepEditor(Context context,
                                          DB_FP * session_fd,
                                          DedEditor * owner);
  virtual			~DepEditor(void);

/* database */
  virtual void			Open(Context context,
		                     Context mod_in_pgm_context,
		                     Context pgm_context,
		                     DB_FP * fd,
                                     DBObject * contents);
  virtual void			Save(Context context, DB_FP * fd);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* viewing */
  virtual View *		OpenView(Context context, DB_FP * session_fd);
  virtual void			CloseView(View * view);
  virtual void			SetView(View * view);

/* filtering */
  virtual UserFilterDef *	OpenFilterDef(Context context,
				              Context mod_in_pgm_context,
				              Context pgm_context,
				              DB_FP * session_fd);
  virtual void			AddStandardFilterDefs(FilterDefSet * defs);

/* input handling */
  virtual Boolean		MenuChoice(Generic cmd);

/* selection */
  virtual void			SetSelection(int line1, int char1, int line2, int char2);
  virtual void			GetNavigationTarget(int line, int col, FortTreeNode &target);

/* access to contents */
  virtual int			NumLines(void);
  virtual int			MaxLineWidth(void);
  virtual void			GetLine(int k, TextString &ts, TextData &td);
  virtual void			SetSortColumn(int colNum);
  virtual int			NumCurrentDependences(void);
  virtual void			GetCurrentDependence(int k, int &depNum);
  virtual void			SetCurrentDependencePolicy(DedCurDepPolicy policy);
  virtual void			GetCurrentDependencePolicy(DedCurDepPolicy &policy);


public:		/* pretend this is 'protected' */

/* initialization */
  virtual void			setContents(DBObject * contents);

/* menu commands */
  virtual void			arrowCommand(void);

};




#endif /* __cplusplus */

#endif /* not DepEditor_h */
