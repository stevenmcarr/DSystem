/* $Id: MenuBar.h,v 1.4 1997/03/11 14:32:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/MenuBar.h						*/
/*									*/
/*	MenuBar -- MenuBar Class					*/
/*	Last edited: October 13, 1993 at 10:13 pm			*/
/*									*/
/************************************************************************/




#ifndef MenuBar_h
#define MenuBar_h


#include <libs/graphicInterface/framework/framework.h>

#include <libs/graphicInterface/framework/CMenu.h>



/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*****************/
/* MenuBar class */
/*****************/


struct MenuBar_Repr_struct;




class MenuBar: public Object
{
public:

  MenuBar_Repr_struct * MenuBar_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(MenuBar)
				MenuBar(void);
  virtual			~MenuBar(void);

/* menus */
  virtual void			AddMenu(char * name, int relPos, char * existingName);
  virtual void			RemoveMenu(char * name);
  virtual int			NumMenus(void);
  virtual char *		GetMenuName(int k);
  virtual CMenu *		GetMenu(int k);

/* tiling */
  virtual void			GetSizePrefs(Point &minSize, Point &defSize);
  virtual Generic		GetTiling(Boolean init, Point size);
  virtual void			InitPanes(void);

/* interaction */
  virtual Boolean		Select(int menuNum, Generic &id);
  virtual Boolean		Keystroke(KbChar kb, Generic &id);

/* TEMPORARY */
  virtual Generic		getButtonPane(void);

};




#endif /* __cplusplus */

#endif /* not MenuBar_h */
