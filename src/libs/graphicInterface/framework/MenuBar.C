/* $Id: MenuBar.C,v 1.5 1997/03/11 14:32:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/MenuBar.C						*/
/*									*/
/*	MenuBar -- MenuBar Class					*/
/*	Last edited: October 13, 1993 at 10:13 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/MenuBar.h>

#include <libs/graphicInterface/oldMonitor/include/mon/cp.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/button_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>





/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* MenuBar object */

typedef struct MenuBar_Repr_struct
  {
    /* menus */
      int       numMenus;
      CMenu * *  menus;		/* array of CMenu * */
      char * *  menuTitles;	/* array of char * */

    /* tiling */
      Generic   buttonPane;

  } MenuBar_Repr;


#define R(ob)		(ob->MenuBar_repr)






/*************************/
/*  Miscellaneous	 */
/*************************/




/* font to use for menu bar */

short MenuBar_buttonFont;






/*************************/
/*  Forward declarations */
/*************************/




/* none */






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void MenuBar::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(CMenu);

  /* font for menu bar */
    MenuBar_buttonFont = DEF_FONT_ID;
}




void MenuBar::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(MenuBar)




MenuBar::MenuBar(void)
{
  /* allocate instance's private data */
    this->MenuBar_repr = (MenuBar_Repr *) get_mem(sizeof(MenuBar_Repr),
                                                  "MenuBar instance");

  /* create the subparts */
    /* menus -- none yet */
      R(this)->numMenus   = 0;
      R(this)->menus      = (CMenu * *) get_mem(0, "MenuBar's menu list");
      R(this)->menuTitles = (char * *) get_mem(0, "MenuBar's menu title list");
}




MenuBar::~MenuBar()
{
  int k;

  /* destroy the menus */
    for( k = 0;  k < R(this)->numMenus;  k++ )
      delete R(this)->menus[k];

  /* destroy the menu lists */
    free_mem((void*) R(this)->menus);
    free_mem((void*) R(this)->menuTitles);
    /* title strings were not copied so should not be destroyed */

  /* destroy instance's private data */
    free_mem((void*) this->MenuBar_repr);
}







/***********/
/*  Menus  */
/***********/





void MenuBar::AddMenu(char * title, int relPos, char * existingTitle)
{
  /* NOTE: 'relPos' and 'existingTitle' are not implemented */

  R(this)->numMenus += 1;

  /* add a new menu with the given title */
    R(this)->menus = (CMenu * *) reget_mem((void*) R(this)->menus,
                                          R(this)->numMenus * sizeof(CMenu *),
                                          "MenuBar's menu list"
                                         );
    R(this)->menus[R(this)->numMenus-1] = new CMenu(title);

    /* add the title to title list */
      R(this)->menuTitles = (char * *) reget_mem((void*) R(this)->menuTitles,
                                                 R(this)->numMenus * sizeof(char *),
                                                 "MenuBar's menu title list"
                                                );
    R(this)->menuTitles[R(this)->numMenus-1] = title;
}




void MenuBar::RemoveMenu(char * title)
{
  NOT_IMPLEMENTED("MenuBar::RemoveMenu");
}




int MenuBar::NumMenus(void)
{
  return R(this)->numMenus;
}




char * MenuBar::GetMenuName(int k)
{
  return R(this)->menuTitles[k];
}




CMenu * MenuBar::GetMenu(int k)
{
  return R(this)->menus[k];
}






/************/
/*  Tiling  */
/************/




void MenuBar::GetSizePrefs(Point &minSize, Point &defSize)
{
  static char * titles[1] = { " " };

  if( R(this)->numMenus > 0 )
    defSize = sm_button_pane_size(makePoint(R(this)->numMenus, 1),
                                  R(this)->menuTitles,
                                  MenuBar_buttonFont);
  else
    defSize = sm_button_pane_size(makePoint(1,1),
                                  titles,
                                  MenuBar_buttonFont);

  minSize.x = 72;
  minSize.y = defSize.y;
}




Generic MenuBar::GetTiling(Boolean init, Point size)
{
  if( init )
    R(this)->buttonPane = ( R(this)->numMenus > 0 ? sm_button_get_index()
                                                  : sm_vanilla_get_index() );

  return cp_td_pane((Pane**)&R(this)->buttonPane, size);
}




void MenuBar::InitPanes(void)
{
  if( R(this)->numMenus > 0 )
    sm_button_create_btns((Pane *) R(this)->buttonPane,
                          makePoint(R(this)->numMenus, 1),
                          R(this)->menuTitles,
                          MenuBar_buttonFont,
                          false);
}







/*****************/
/*  Interaction  */
/*****************/




/* TEMPORARY */

Boolean MenuBar::Select(int menuNum, Generic &id)
{
  /* do something about preparing menus? */

  return R(this)->menus[menuNum]->Select(id);
}




Boolean MenuBar::Keystroke(KbChar kb, Generic &id)
{
  /* TEMPORARY */
  return false;
}





/* TEMPORARY */

Generic MenuBar::getButtonPane(void)
{
  return R(this)->buttonPane;
}








/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
