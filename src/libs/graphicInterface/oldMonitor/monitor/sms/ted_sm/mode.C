/* $Id: mode.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>


void sm_ted_octal_mode(Pane *p)
{
	CTRL_STYLE(p) = OCTAL;

	sm_ted_damaged_win(p);
}


void sm_ted_caret_mode(Pane *p)
{
	CTRL_STYLE(p) = CARET;

	sm_ted_damaged_win(p);
}


void sm_ted_verbatim_mode(Pane *p)
{
	CTRL_STYLE(p) = VERBATIM;

	sm_ted_damaged_win(p);
}
