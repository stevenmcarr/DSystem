/* $Id: CMenu.C,v 1.2 1997/03/11 14:32:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/CMenu.C						*/
/*									*/
/*	CMenu -- CMenu Class						*/
/*	Last edited: October 13, 1993 at 12:32 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/CMenu.h>

#include <libs/graphicInterface/oldMonitor/include/mon/menu.h>





/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* CMenu object */

typedef struct Menu_Repr_struct
  {
    /* subparts */
      aMenuDef * def;
      aMenu *    menu;

  } Menu_Repr;


#define R(ob)		(ob->Menu_repr)





/*************************/
/*  Forward declarations */
/*************************/




static void ensureMenu(CMenu * m);
static void destroyMenu(CMenu * m);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void CMenu::InitClass(void)
{
  /* ... */
}




void CMenu::FiniClass(void)
{
  /* ... */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(CMenu)




CMenu::CMenu(char * title)
{
  /* allocate instance's private data */
    this->Menu_repr = (Menu_Repr *) get_mem(sizeof(Menu_Repr), "CMenu instance");

  /* create the subparts */
    /* menu def */
      R(this)->def = (aMenuDef *) get_mem(sizeof(aMenuDef), "CMenu's aMenuDef");
      R(this)->def->title = title;
      R(this)->def->size  = makePoint(1, 0);
      R(this)->def->def   = UNUSED;

    /* choice list */
      /* no choices yet */
      R(this)->def->choice_list = (aChoiceDef *) get_mem(0, "CMenu's aChoiceDef list");

    /* menu is created lazily */
      R(this)->menu = nil;
}




CMenu::~CMenu()
{
  aMenuDef * def = R(this)->def;
  int k;

  /* destroy the menu definition */
    /* the various strings were not copied so should not be destroyed */
    for( k = 0;  k < def->size.y;  k++ )
      free_mem((void*) def->choice_list[k].option_list);
    free_mem((void*) def->choice_list);
    free_mem((void*) def);

  /* destroy the menu object */
    destroyMenu(this);

  /* destroy instance's private data */
    free_mem((void*) this->Menu_repr);
}







/****************/
/*  CMenu items  */
/****************/




void CMenu::AddItem(Generic id,
                   KbChar key,
                   char * text,
                   char * help_text,
                   int relPos,
                   Generic existingId)
{
  aMenuDef * def = R(this)->def;
  aChoiceDef choice;
  anOptionDef * option;

  /* NOTE: 'relPos' and 'existingId' are not implemented */

  destroyMenu(this);

  /* make a new option, dynamically allocated */
    option = (anOptionDef *) get_mem(sizeof(anOptionDef), "CMenu's anOptionDef");
    option->displayed_text = text;
    option->help_text      = help_text;

  /* make a new choice containing the option, in a local */
    choice.id          = id;
    choice.kb_code     = key;
    choice.num_options = 1;
    choice.option_list = option;

  /* make room for the new choice in the menu definition & insert it */
    def->size.y += 1;
    def->choice_list = (aChoiceDef *) reget_mem((void*) def->choice_list,
                                                def->size.y * sizeof(aChoiceDef),
                                                "CMenu's aChoiceDef list"
                                               );
    def->choice_list[def->size.y - 1] = choice;
}




void CMenu::RemoveItem(Generic id)
{
  NOT_IMPLEMENTED("CMenu::RemoveItem");
}




void CMenu::ChangeItem(Generic id, char * text)
{
  NOT_IMPLEMENTED("CMenu::ChangeItem");
}




void CMenu::EnableItem(Generic id, Boolean enabled)
{
  NOT_IMPLEMENTED("CMenu::EnableItem");
}





void CMenu::CheckItem(Generic id, Boolean checked)
{
  /* TEMPORARY */
  ensureMenu(this);
  modify_menu_choice(R(this)->menu, id, true, checked);
}





/*****************/
/*  Interaction  */
/*****************/




Boolean CMenu::Select(Generic &id)
{
  ensureMenu(this);
  id = select_from_menu(R(this)->menu, false);
  return BOOL( id != UNUSED );
}







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* Lazy menu creation */


static
void ensureMenu(CMenu * m)
{
  if( R(m)->menu == nil )
    R(m)->menu = create_menu(R(m)->def);
}




static
void destroyMenu(CMenu * m)
{
  if( R(m)->menu != nil )
    { destroy_menu(R(m)->menu);
      R(m)->menu = nil;
    }
}
