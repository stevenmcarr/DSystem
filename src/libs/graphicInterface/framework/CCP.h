/* $Id: CCP.h,v 1.2 1997/03/11 14:32:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/CCP.h							*/
/*									*/
/*	CP -- Command Processor Class					*/
/*	Last edited: November 10, 1993 at 2:35 pm			*/
/*									*/
/************************************************************************/




#ifndef CP_h
#define CP_h


#include <libs/graphicInterface/framework/framework.h>


#include <libs/graphicInterface/oldMonitor/include/mon/cp.h>
#include <libs/graphicInterface/framework/MenuBar.h>
#include <libs/graphicInterface/framework/CMenu.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/************/
/* CP class */
/************/


struct CP_Repr_struct;
class  View;




class CP: public Object
{
public:

  CP_Repr_struct * CP_repr;


public:

/* class initialization */
  STATIC(void,			InitClass, (void));
  STATIC(void,			FiniClass, (void));

/* initialization */
  META_DEF(CP)
				CP(Generic parent_id, Generic cp_id);
  virtual			~CP(void);
  virtual void			Init(void);
  virtual void			Fini(void);

  static CP *			CurrentCP;

/* raw interface to Rn monitor */
  virtual Boolean		CP_HandleInput(Generic generator,
                                               short event_type,
                                               Point info,
                                               Generic msg);

/* window layout */
  virtual char *		GetWindowTitle(void);
  virtual void			SetWindowTitle(char * title);
  virtual void			GetContentsSizePrefs(Point &minSize, Point &defSize);
  virtual Generic		GetContentsTiling(Boolean init, Point contentsSize);
  virtual void			InitContentsPanes(void);
  virtual void			RetileWindow(void);

/* menus */
  virtual void			InitMenuBar(MenuBar * mb);
  virtual void			InitMenu(char * name, CMenu * m);

/* input handling */
  virtual void			Message(Generic msg);
  virtual void			Resize(Point newSize);
  virtual Boolean		QueryKill(void);
  virtual void			Help(Point pt);
  virtual char *		GetHelpFileName(void);
  virtual void			SelectionEvent(Generic generator, Point info);
  virtual Boolean		MenuChoice(Generic cmd);
  virtual void			Keystroke(KbChar kb);

/* window manipulation */
  virtual void			WindowToTop(void);
  static  void			CurrentWindowToTop(void);

/* subordinate windows */
  virtual Window *		OpenAuxWindow(View * view, char * title, Boolean visible);
  virtual void			CloseAuxWindow(Window * window);
  virtual void			ShowAuxWindow(Window * window);
  virtual void			HideAuxWindow(Window * window);
  virtual void			ResizeAuxWindow(Window * window, Point info);


protected:

  /* subordinate windows */
    virtual void		addAuxWindowInfo(Window * window, View * view);
    virtual int			findAuxWindowInfo(Window * window, View * &view);
    virtual void		removeAuxWindowInfo(Window * window);

};






/******************/
/* CP declaration */
/******************/




#define CP_DECLARATION(Class, text, onMenu, proc, startup)		\
									\
    static								\
    Generic Class##__CreateInstance(Generic parent_id,			\
                                  Generic cp_id,			\
                                  Generic startup)			\
    {									\
      Class * cp;							\
									\
      cp = new Class(parent_id, cp_id, (void *) startup);	        \
      cp->Init();							\
      return (Generic) cp;						\
    }									\
									\
    static								\
    Boolean Class##__Init(Generic mgr)					\
    {									\
      REQUIRE_INIT(Class);						\
      return true;							\
    }									\
									\
    static								\
    void Class##__Fini(void)						\
    {									\
      Object::PerformNotedFinis();					\
    }									\
									\
    extern Boolean CP_HandleInput(Generic,Generic,short,Point,Generic);	\
    extern void CP_DestroyInstance(Generic, Boolean);			\
									\
									\
    aProcessor proc =							\
      {									\
        text,								\
        onMenu,								\
        (Generic)&startup,						\
        (cp_root_starter_func)cp_standard_root_starter,					\
        Class##__Init,							\
        Class##__CreateInstance,					\
        (cp_handle_input_func)CP_HandleInput,							\
        (cp_destroy_instance_func)CP_DestroyInstance,						\
        Class##__Fini,							\
        CP_UNSTARTED							\
      };




#endif /* __cplusplus */

#endif /* not CP_h */
