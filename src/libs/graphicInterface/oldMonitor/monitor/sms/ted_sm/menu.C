/* $Id: menu.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>

/* these functions exist to allow ../ted_sm/bind.c to name the
 * same functional interface as the menus in ../ted_cp/ted.c 
 *
 *						- kdc 30 Sept 88
 */

Boolean sm_ted_menu_copy(Pane *p)
{
  sm_ted_copy_from_dot_to_mark(p);
  sm_ted_insert_copy_buffer(p);
  sm_ted_inform(p, "Copied selection.");
  return true;
}

Boolean sm_ted_menu_cut(Pane *p)
{
  sm_ted_copy_from_dot_to_mark(p);
  sm_ted_delete_from_dot_to_mark(p);
  return true;
}

Boolean sm_ted_menu_find(Pane *p)
{
  (void) sm_ted_find_dialog_run_find(p);
  return true;
}

Boolean sm_ted_menu_replace(Pane *p)
{
  (void) sm_ted_find_dialog_run_replace(p);
  return true;
}
