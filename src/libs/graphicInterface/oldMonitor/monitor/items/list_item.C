/* $Id: list_item.C,v 1.1 1997/06/25 14:48:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	items/list.c							*/
/*									*/
/*	list -- dialog item made out of the list screen module		*/
/*	Last edited: May 25, 1990 at 5:40 pm				*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/oldMonitor/include/mon/item_def.h>
#include <libs/graphicInterface/oldMonitor/include/items/item_list.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/list_sm.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/



typedef struct
  {
    Generic          owner;
    Point            size;
    char *           title;
    short            font;
    item_list_elem_proc         listElemProc;
    Generic          *vptr;
    Generic          null_id;
    Boolean          toggle;
    Generic          selectedLine;
    Pane *           title_pane;
    Pane *           list_pane;
   
  } list_item_Repr;


#define	R(dd)		((list_item_Repr *) dd->item_info)




/*****************************************/
/*  Early declarations for method procs  */
/*****************************************/


STATIC(Point,        get_size,(DiaDesc *dd));
STATIC(void,         initialize,(DiaDesc *dd, Point ulc));
STATIC(FocusStatus,  handle_event,(DiaDesc *dd));
STATIC(void,         modified,(DiaDesc *dd, Boolean user));
STATIC(void,         destroy,(DiaDesc *dd));




/*************************/
/*  Item definition      */
/*************************/


static Item list_item =
  {
    "list",
    get_size,
    initialize,
    handle_event,
    standardNoFocusSetter,
    modified,
    destroy
  };




/*************************/
/*  Forward declarations */
/*************************/


STATIC(lsm_line_info*, stdListElemProc,(Pane *p, DiaDesc *dd, Boolean request, 
                                        Generic id));










/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




DiaDesc * item_list(Generic item_id, char *title, Generic owner, 
                    item_list_elem_proc listElemProc, Generic *vptr, 
                    Generic null_id, Boolean toggle, short font, Point size)
{
  list_item_Repr * repr;

  if( title == (char *) 0 )
    title = "";

  /* allocate a new instance */
    repr = (list_item_Repr *) get_mem(sizeof(list_item_Repr),"ListItem");
    repr->owner        = owner;
    repr->size         = size;
    repr->title        = ssave(title);
    repr->font         = font;
    repr->null_id      = null_id;
    repr->toggle       = toggle;
    repr->listElemProc = listElemProc;
    repr->vptr         = vptr;
    repr->title_pane   = NULL_PANE;
    repr->list_pane    = NULL_PANE;

  return makeDialogDescriptor(&list_item, (Generic) repr, item_id);
}




ListItemEntry item_list_create_entry(DiaDesc *dd, Generic id, char *text, 
                                     Boolean selectable)
{
  lsm_line_info * line;

  line = (lsm_line_info *) get_mem(sizeof(lsm_line_info), "list item: new line");
  line->text        = ssave(text);
  line->should_free = true;
  line->len         = strlen(text);
  line->id          = id;
  line->selected    = BOOL( line->id == R(dd)->selectedLine );
  line->selectable  = selectable;
  return (ListItemEntry) line;
}




void item_list_modified(DiaDesc *dd)
{
  R(dd)->selectedLine = *R(dd)->vptr;
  (void) sm_list_modified(R(dd)->list_pane,makePoint(UNUSED,UNUSED),UNUSED);
  dialog_item_redraw_pane(dd, R(dd)->list_pane);
}




void item_list_line_modified(DiaDesc *dd, Generic id)
{
  sm_list_line_modified(R(dd)->list_pane,id);
  dialog_item_redraw_pane(dd,R(dd)->list_pane);
}




void item_list_line_change(DiaDesc *dd, Generic id, Boolean selectable)
{
  sm_list_line_change(R(dd)->list_pane,id,BOOL(id == R(dd)->selectedLine),
						selectable);
  dialog_item_redraw_pane(dd,R(dd)->list_pane);
}




void item_list_line_show(DiaDesc *dd, Generic id, Boolean New)
{
  (void) sm_list_line_show(R(dd)->list_pane,id,New,UNUSED);
  dialog_item_redraw_pane(dd,R(dd)->list_pane);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
Point get_size(DiaDesc *dd)
{
  Point title_size,list_size,size;

  list_size  = transPoint(
                    sm_list_pane_size(
			R(dd)->size,
			R(dd)->font,
			LSM_NO_H_SCROLLBAR,
			LSM_V_SCROLLBAR),
		    makePoint(2,2));	/* extra border */

  if( strlen(R(dd)->title) == 0 )
    title_size = Origin;
  else
    title_size = sm_vanilla_pane_size(R(dd)->title,R(dd)->font);

  size.x = max(list_size.x,title_size.x);
  size.y = list_size.y + title_size.y;

  return size;
}




static
void initialize(DiaDesc *dd, Point ulc)
{
  Point title_size,list_size;

  /* get and adjust the pane sizes */
    list_size  = transPoint(
                      sm_list_pane_size(R(dd)->size,R(dd)->font,
				LSM_NO_H_SCROLLBAR,LSM_V_SCROLLBAR),
		      makePoint(2,2));	/* extra border */

    if( strlen(R(dd)->title) == 0 )
      title_size = Origin;
    else
      title_size  = sm_vanilla_pane_size(R(dd)->title,R(dd)->font);

    list_size.x = title_size.x = max(list_size.x,title_size.x);

  /* create and initialize the title pane */
    R(dd)->title_pane =
        dialog_item_make_pane(dd, sm_vanilla_get_index(), ulc, title_size, 0);
    sm_vanilla_set_text(R(dd)->title_pane,R(dd)->title,
                        R(dd)->font,STYLE_BOLD,VSM_JUSTIFY_CENTER);

    sfree(R(dd)->title);
    R(dd)->title = (char *) 0;

  /* create and initialize the list pane */
    ulc.y += title_size.y;
    R(dd)->list_pane =
        dialog_item_make_pane(dd,sm_list_get_index(),ulc,list_size,2);
    sm_list_initialize(R(dd)->list_pane,
                       (Generic)dd, (lsm_generator_callback)stdListElemProc, R(dd)->font,
                       LSM_SHIFT_AUTO, LSM_NO_H_SCROLLBAR, LSM_V_SCROLLBAR);

  /* initialize the current selection */
    R(dd)->selectedLine = *R(dd)->vptr;

  sm_list_modified(R(dd)->list_pane,Origin,UNUSED);
}




static
FocusStatus handle_event(DiaDesc *dd)
{
  if( mon_event.type == EVENT_SELECT &&
      mon_event.from == (Generic) R(dd)->list_pane )
    { if( R(dd)->toggle  &&  R(dd)->selectedLine == mon_event.msg )
        R(dd)->selectedLine = R(dd)->null_id;
      else
        R(dd)->selectedLine = mon_event.msg;
      dialog_item_user_change(dd);
    }

  return FocusOK;
}




static
void modified(DiaDesc *dd, Boolean user)
{
  if( user )
    { /* selection from list_sm adjusts dialog item value */
      *R(dd)->vptr = R(dd)->selectedLine;
    }
  else
    { /* dialog item value adjusts selection from list_sm */
      R(dd)->selectedLine = *R(dd)->vptr;
    }
  sm_list_modified(R(dd)->list_pane,makePoint(UNUSED,UNUSED),UNUSED);
}




static
void destroy(DiaDesc *dd)
{
  free_mem((void*)dd->item_info);
}




/*ARGSUSED*/

static
lsm_line_info * stdListElemProc(Pane *p, DiaDesc *dd, Boolean request, Generic id)
{
  lsm_line_info * line;

  line = (lsm_line_info *)((R(dd)->listElemProc)(R(dd)->owner,dd,request,id));
  return line;
}





