/* $Id: Editor.h,v 1.6 1997/03/11 14:32:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Editor.h						*/
/*									*/
/*	Editor -- Abstract class for all Editors			*/
/*	Last edited: November 6, 1993 at 10:21 pm			*/
/*									*/
/************************************************************************/




#ifndef Editor_h
#define Editor_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/DBObject.h>
#include <libs/graphicInterface/framework/View.h>
#include <libs/graphicInterface/framework/Selection.h>
#include <libs/graphicInterface/framework/Scrap.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/****************/
/* Editor class */
/****************/




struct Editor_Repr_struct;
class View;
class UserFilterDef;
class FilterDefSet;


class Editor: public Object
{
public:

  Editor_Repr_struct * Editor_repr;


/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(Editor)
				Editor(Context context, DB_FP * session_fp);
  virtual			~Editor(void);

/* database */
  virtual void			Open(Context context,
		                     Context mod_in_pgm_context,
		                     Context pgm_context,
		                     DB_FP * fp, DBObject * contents);
  virtual void			Save(Context context, DB_FP * fp);
  virtual void			Close(void);

/* viewing */
  virtual View *		OpenView(Context context, DB_FP * session_fp);
  virtual void			CloseView(View * view);
  virtual void			SetView(View * view);

/* filtering */
  virtual UserFilterDef *	OpenFilterDef(Context context,
				              Context mod_in_pgm_context,
				              Context pgm_context,
				              DB_FP * session_fp);
  virtual void			AddStandardFilterDefs(FilterDefSet * defs);
  virtual void			GetFilterDefByName(char * name, UserFilterDef * &def);
  virtual void			DefineFilters(void);
  virtual void			ChooseFilterFor(View * view);

/* input handling */
  virtual Boolean		MenuChoice(Generic id);
  virtual void			Keystroke(KbChar kb);

/* change notification */
  virtual void			BeginEdit(void);
  virtual void			EndEdit(void);
  virtual Boolean		Frozen(void);
  virtual void			Changed(int kind, void * change);

/* selection */
  virtual void			GetSelection(Selection * &sel);
  virtual void			SetSelection(Selection * sel);

/* editing */
  virtual void			Copy(void);
  virtual void			Cut(void);
  virtual void			Paste(void);
  virtual void			Clear(void);
  virtual Scrap *		Extract(Selection * sel);
  virtual void			Replace(Selection * sel, Scrap * scrap);


public:		/* pretend this is 'private' */

/* initialization */
  virtual DBObject *		openContents(Context context,
			                     Context mod_in_pgm_context,
			                     Context pgm_context,
			                     DB_FP * session_fp);
  virtual void			setContents(DBObject * contents);
  virtual DBObject *		getContents(void);

/* change notification */
  virtual void			clearDeferredChanges(void);
  virtual void			addDeferredChange(int kind, void * change);
  virtual void			getDeferredChanges(int &kind, void * &change);

};




/****************************/
/* Standard editor commands */
/****************************/


/*  "file" menu commands */

#define CMD_NEW			1001
#define CMD_EDIT		1002
#define CMD_BROWSE		1003
#define CMD_SAVE_AS		1004
#define CMD_SAVE_COPY		1005
#define CMD_SAVE		1006


/* "edit" menu commands */

#define CMD_UNDO		1021
#define CMD_COPY		1022
#define CMD_CUT			1023
#define CMD_PASTE		1024
#define CMD_CLEAR		1025
#define CMD_SELECT_ALL		1026


/* "view" menu commands */

#define CMD_DEF_FILTER		1041
#define CMD_SET_FILTER		1042
#define CMD_SPLIT_VIEW		1043
#define CMD_SHOW_SEL		1044


/* "search" menu commands */

#define CMD_FIND		1061
#define CMD_FIND_NEXT		1062
#define CMD_FIND_PREV		1063
#define CMD_REPLACE		1064
#define CMD_GENERAL_INFO	1065
#define CMD_SELECTED_INFO	1066




#endif /* __cplusplus */

#endif /* not Editor_h */
