/* $Id: EditorCP.h,v 1.6 1997/03/11 14:32:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/EditorCP.h						*/
/*									*/
/*	EditorCP -- Editor-like Command Processor			*/
/*	Last edited: October 13, 1993 at 5:56 pm			*/
/*									*/
/************************************************************************/




#ifndef EditorCP_h
#define EditorCP_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/CCP.h>
#include <libs/graphicInterface/framework/Editor.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/******************/
/* EditorCP class */
/******************/




struct EditorCP_Repr_struct;




class EditorCP: public CP
{
public:

  EditorCP_Repr_struct * EditorCP_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(EditorCP)
				EditorCP(Generic parent_id, Generic EditorCP_id, void * startup);
  virtual			~EditorCP(void);
  virtual void			Init(void);
  virtual void			Fini(void);


/* window layout */
  virtual char *		GetWindowTitle(void);
  virtual char *		GetWindowTitleFormat(void);
  virtual void			GetContentsSizePrefs(Point &minSize, Point &defSize);
  virtual Generic		GetContentsTiling(Boolean init, Point contentsSize);
  virtual void			InitContentsPanes(void);


/* menus */
  virtual void			InitMenuBar(MenuBar * mb);
  virtual void			InitMenu(char * name, CMenu * m);

/* input handling */
  virtual Boolean		QueryKill(void);
  virtual void			SelectionEvent(Generic generator, Point info);
  virtual Boolean		MenuChoice(Generic id);
  virtual void			Keystroke(KbChar kb);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* database */
  virtual void			GetSessionAttribute(char * &session_attr,
                                                    int &oldest,
                                                    int &newest);
  virtual  void			GetContentsAttribute(char * &contents_attr);
  virtual void			Open(Context mod_context,
		                     Context mod_in_pgm_context,
		                     Context pgm_context,
		                     DB_FP * fp);
  virtual void			Close(void);
  virtual void			Save(Context mod_context, DB_FP * fp);


protected:

/* initialization */
  virtual Editor *		openEditor(Context mod_context,
			                   Context mod_in_pgm_context,
			                   Context pgm_context,
			                   DB_FP * session_fp);
  virtual void			closeEditor(void);
  View *			getView(void);

};




/********************/
/* Startup argument */
/********************/




typedef struct
  {    
    Context mod_context;
    Context mod_in_pgm_context;
    Context pgm_context;

  } EditorCPStartupStruct;

extern EditorCPStartupStruct EditorCP_startup;




#endif /* __cplusplus */

#endif /* not EditorCP_h */
