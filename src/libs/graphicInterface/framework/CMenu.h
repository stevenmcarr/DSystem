/* $Id: CMenu.h,v 1.2 1997/03/11 14:32:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/CMenu.h						*/
/*									*/
/*	CMenu -- Menu Class						*/
/*	Last edited: October 13, 1993 at 12:32 pm			*/
/*									*/
/************************************************************************/




#ifndef Menu_h
#define Menu_h


#include <libs/graphicInterface/framework/framework.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/***************/
/* CMenu class */
/***************/




struct Menu_Repr_struct;




class CMenu: public Object
{
public:

  Menu_Repr_struct * Menu_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(CMenu)
				CMenu(char * title);
  virtual			~CMenu(void);

/* menu items */
  virtual void			AddItem(Generic id,
                                        KbChar key,
                                        char * text,
                                        char * help_text,
                                        int relPos,
                                        Generic existingId);
  virtual void			RemoveItem(Generic id);
  virtual void			ChangeItem(Generic id, char * text);
  virtual void			EnableItem(Generic id, Boolean enabled);
  virtual void			CheckItem(Generic id, Boolean checked);

/* interaction */
  virtual Boolean		Select(Generic &id);

};




#endif /* __cplusplus */

#endif /* not Menu_h */
