/* $Id: DedEditor.h,v 1.2 1997/03/11 14:30:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/DedEditor.h						*/
/*									*/
/*	DedEditor -- Fortran D Editor					*/
/*	Last edited: November 12, 1993 at 4:22 pm			*/
/*									*/
/************************************************************************/




#ifndef DedEditor_h
#define DedEditor_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/SplitEditor.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>

#include <libs/frontEnd/fortTree/FortTree.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*******************/
/* DedEditor class */
/*******************/




struct DedEditor_Repr_struct;




class DedEditor: public SplitEditor
{
public:

  DedEditor_Repr_struct * DedEditor_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(DedEditor)
				DedEditor(Context context, DB_FP * session_fd);
  virtual			~DedEditor(void);

/* database */
  virtual void			GetContentsAttribute(char * &contents_attr);
  virtual void                  Open(Context context,
                                     Context mod_in_pgm_context,
                                     Context pgm_context,
                                     DB_FP * session_fd,
                                     DBObject * contents);

/* viewing */
  virtual View *		OpenView(Context context, DB_FP * session_fd);

/* input handling */
  virtual Boolean		MenuChoice(Generic cmd);
  virtual void                  Keystroke(KbChar kb);

/* global editing state */
  virtual void			SetCurrentLoop(FortTreeNode loop);
  virtual void			GetCurrentLoop(FortTreeNode &loop);
  virtual int			NumCurrentDependences(void);
  virtual void			GetCurrentDependence(int k, int &depNum);
  virtual void			SetCurrentDependencePolicy(DedCurDepPolicy policy);
  virtual void			GetCurrentDependencePolicy(DedCurDepPolicy &policy);

/* selection */
  virtual void			SetSourceSelection(int line1, int char1, int line2, int char2);
/* navigation */
  virtual void			NavigateTo(FortTreeNode target);


protected:

/* initialization */
  virtual DBObject *		openContents(Context context,
			                     Context mod_in_pgm_context,
			                     Context pgm_context,
			                     DB_FP * session_fd);
  virtual void			setContents(DBObject * contents);
  virtual void			addSubEditors(Flex * subEditors,
                                              Flex * subEditorCaptions,
                                              Editor * &mainSubEditor,
                                              Context context,
                                              DB_FP * session_fd);

};




/**************************/
/* DedEditor menu choices */
/**************************/


/* edit */

#define CMD_ANALYZE			2001


/* view */

#define CMD_DEF_NAV_FILTER		2101
#define CMD_DEF_SRC_FILTER		2102
#define CMD_DEF_DEP_FILTER		2103
#define CMD_DEF_COMM_FILTER		2104
#define CMD_DEF_DIST_FILTER		2105

#define CMD_SET_NAV_FILTER		2111
#define CMD_SET_SRC_FILTER		2112
#define CMD_SET_DEP_FILTER		2113
#define CMD_SET_COMM_FILTER		2114
#define CMD_SET_DIST_FILTER		2115

#define CMD_ARROWS			2116




#endif /* __cplusplus */

#endif /* not DedEditor_h */
