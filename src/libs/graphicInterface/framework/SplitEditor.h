/* $Id: SplitEditor.h,v 1.2 1997/03/11 14:32:53 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/SplitEditor.h						*/
/*									*/
/*	SplitEditor -- Editor made of sub-editors			*/
/*	Last edited: November 10, 1993 at 8:16 pm			*/
/*									*/
/************************************************************************/




#ifndef SplitEditor_h
#define SplitEditor_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/Editor.h>

#include <libs/support/arrays/FlexibleArray.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




enum SplitEditor_Visibility
{
  SPLIT_IN_PANE,
  SPLIT_IN_WINDOW,
  SPLIT_HIDDEN
};




/*********************/
/* SplitEditor class */
/*********************/




struct SplitEditor_Repr_struct;
class  DBObject;
class  View;




class SplitEditor: public Editor
{
public:

  SplitEditor_Repr_struct * SplitEditor_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(SplitEditor)
				SplitEditor(Context context, DB_FP * session_fd);
  virtual			~SplitEditor(void);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* database */
  virtual void			Open(Context context,
		                     Context mod_in_pgm_context,
		                     Context pgm_context,
		                     DB_FP * session_fd,
                                     DBObject * contents);
  virtual void			Close(void);
  virtual void			Save(Context context, DB_FP * session_fd);

/* viewing */
  virtual View *		OpenView(Context context, DB_FP * session_fd);
  virtual void			SetSubviewVisibility(int k, SplitEditor_Visibility vis);

/* input handling */
  virtual Boolean		MenuChoice(Generic cmd);
  virtual void			Keystroke(KbChar kb);

public:		/* pretend this is 'protected'! */

/* initialization */
  virtual void			setContents(DBObject * contents);
  virtual void			addSubEditors(Flex * subEditors,
                                              Flex * subEditorCaptions,
                                              Editor * &mainSubEditor,
                                              Context context,
                                              DB_FP * session_fd);

};




#endif /* __cplusplus */

#endif /* not SplitEditor_h */
