/* $Id: DedCP.h,v 1.3 1997/06/24 17:56:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/DedCP.h							*/
/*									*/
/*	DedCP -- Ded Command Processor					*/
/*	Last edited: November 10, 1993 at 11:51 pm			*/
/*									*/
/************************************************************************/




#ifndef DedCP_h
#define DedCP_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/EditorCP.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedEditor.h>




/************************************************************************/
/*	Procedural Interface						*/
/************************************************************************/


EXTERN (int, dedcp_Edit, (int argc, char **argv));




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/***************/
/* DedCP class */
/***************/




/* DedCP object */

typedef struct DedCP_Repr_struct
  {
    DedEditor *	editor;

  } DedCP_Repr;





class DedCP: public EditorCP
{
public:

  DedCP_Repr *DedCP_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);

/* initialization */
  META_DEF(DedCP)
				DedCP(Generic parent_id, Generic DedCP_id, void * startup);
  virtual			~DedCP(void);
  virtual void			Init(void);
  virtual void			Fini(void);

/* database */
  virtual void			GetSessionAttribute(char * &session_attr,
                                                    int &oldest,
                                                    int &newest);

/* window layout */
  virtual char *		GetWindowTitleFormat(void);

/* subordinate windows */
  virtual void			CloseAuxWindow(Window * window);

/* menus */
  virtual void			InitMenuBar(MenuBar * mb);
  virtual void			InitMenu(char * name, CMenu * m);

/* input handling */
  virtual Boolean		MenuChoice(Generic id);


protected:

/* initialization */
  virtual Editor *		openEditor(Context context,
			                   Context mod_in_pgm_context,
			                   Context pgm_context,
			                   DB_FP * session_fd);
  virtual void			closeEditor(void);
  
};




#endif /* __cplusplus */

#endif /* not DedCP_h */
