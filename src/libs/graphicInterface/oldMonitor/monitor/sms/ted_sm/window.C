/* $Id: window.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>

/*
 *  sm_ted_refresh forces a redraw of the current window, regardless of whether it needs it.
 */
void sm_ted_refresh(Pane *p)
{
	sm_ted_damaged_win(p);
}

/*
 *  sm_ted_figure_region is the call back for interactive region selection.
 */
