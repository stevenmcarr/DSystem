/* $Id: llib-lfor_mgrs.c,v 1.4 1997/03/11 14:33:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* LINTLIBRARY */
/* Lint library for managers */

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/mon/manager.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm.h>


/**************************** EVENT CALLBACKS ***************************/

void		sm_desk_register_fd(fd, mid, buf)		short fd; Generic mid; Boolean buf; {}
void		sm_desk_unregister_fd(fd, mid)			short fd; Generic mid; {}
void		sm_desk_register_process(pid, mid)		int pid; Generic mid; {}
void		sm_desk_unregister_process(pid, mid)		int pid; Generic mid; {}
void		sm_desk_register_option(id, name, help, mid)	Generic id; char *name; char *help; Generic mid; {}
void		sm_desk_unregister_option(id, mid)		Generic id; Generic mid; {}


/************************** WINDOW CALLBACKS ****************************/

Window		*sm_desk_win_create(mid, info, font, resize)	Generic mid; Generic info; short font; Boolean resize; {return NULL_WINDOW;}
void		sm_desk_win_modify(w, info)			Window *w; Generic info; {}
void		sm_desk_win_destroy(w)				Window *w; {}
void		sm_desk_win_show(w)				Window *w; {}
void		sm_desk_win_hide(w)				Window *w; {}
void		sm_desk_win_top(w)				Window *w; {}
void		sm_desk_win_force(w, cur)			Window *w; MouseCursor cur; {}
void		sm_desk_win_release(w)				Window *w; {}
short		sm_desk_title_width(title, font)		char *title; short font; {return 0;}
void		sm_desk_win_title(w, title)			Window *w; char *title; {}
Boolean		sm_desk_owns_pane(p)				Pane *p; {return false;}
