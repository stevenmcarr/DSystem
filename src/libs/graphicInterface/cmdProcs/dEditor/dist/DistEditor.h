/* $Id: DistEditor.h,v 1.2 1997/03/11 14:30:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/dist/DistEditor.h					*/
/*									*/
/*	DistEditor -- Ded Variable distribution sub-editor		*/
/*	Last edited: November 7, 1993 at 10:36 pm			*/
/*									*/
/************************************************************************/




#ifndef DistEditor_h
#define DistEditor_h


#include <libs/graphicInterface/framework/framework.h>

#include <libs/graphicInterface/framework/ColumnEditor.h>

#include <libs/frontEnd/fortTree/FortTree.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/**********************/
/* DistEditor class */
/**********************/




struct DistEditor_Repr_struct;
class  UserFilterDef;
class  FilterDefSet;
class  DedEditor;




class DistEditor: public ColumnEditor
{
public:
  DistEditor_Repr_struct * DistEditor_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(DistEditor)
				DistEditor(Context context,
                                            DB_FP * session_fd,
                                            DedEditor * owner);
  virtual			~DistEditor(void);

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
  virtual void			SetView(View * view);

/* filtering */
  virtual UserFilterDef *	OpenFilterDef(Context context,
				              Context mod_in_pgm_context,
				              Context pgm_context,
				              DB_FP * session_fd);
  virtual void			AddStandardFilterDefs(FilterDefSet * defs);

/* input handling */
  virtual Boolean		MenuChoice(Generic cmd);

/* access to contents */
  virtual int			NumLines(void);
  virtual int			MaxLineWidth(void);
  virtual void			GetLine(int k, TextString &ts, TextData &td);

/* selection */
  virtual void			SetSelection(int line1, int char1, int line2, int char2);
  virtual void			GetNavigationTarget(int line, int col, FortTreeNode &target);


protected:

/* initialization */
  virtual void			setContents(DBObject * contents);

};




#endif /* __cplusplus */

#endif /* not DistEditor_h */
