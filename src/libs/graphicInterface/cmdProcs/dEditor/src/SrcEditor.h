/* $Id: SrcEditor.h,v 1.2 1997/03/11 14:30:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/src/SrcEditor.h						*/
/*									*/
/*	SrcEditor -- Ded Source sub-editor				*/
/*	Last edited: November 7, 1993 at 10:36 pm			*/
/*									*/
/************************************************************************/




#ifndef SrcEditor_h
#define SrcEditor_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/CFortEditor.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*******************/
/* SrcEditor class */
/*******************/




struct SrcEditor_Repr_struct;
class UserFilterDef;
class FilterDefSet;
class DedEditor;




class SrcEditor: public CFortEditor
{
public:

  SrcEditor_Repr_struct * SrcEditor_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(SrcEditor)
				SrcEditor(Context context,
                                          DB_FP * session_fd,
                                          DedEditor * owner);
  virtual			~SrcEditor(void);
  virtual DedEditor *		GetOwner(void);

/* database */
  virtual void			Open(Context context,
		                     Context mod_in_pgm_context,
		                     Context pgm_context,
		                     DB_FP * fd,
		                     DBObject * contents);

/* change notification */
  virtual void			SetCurrentLoop(FortTreeNode node);

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

/* access to selection */
  virtual void			SetSelection(int line1, int char1, int line2, int char2);


protected:

/* initialization */
  virtual void			setContents(DBObject * contents);

};




#endif /* __cplusplus */

#endif /* not SrcEditor_h */
