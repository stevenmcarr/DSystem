/* $Id: el_view.C,v 1.1 1997/06/25 15:09:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                            		*/
/*	dep/el/el_view.c  		                               	*/
/*                                                            		*/
/*      el_view.c -- contains routines for setting and returning	*/
/*                  which dependences are being viewed        		*/
/*	Routines:							*/
/*		el_get_view_lc ()					*/
/*		el_get_view_control ()					*/
/*		el_get_view_li ()					*/
/*		el_get_view_private ()					*/
/*		el_set_view_lc ()					*/
/*		el_set_view_control ()					*/
/*		el_set_view_li ()					*/
/*		el_set_view_private ()					*/
/*									*/
/************************************************************************/


#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>
#include <libs/moduleAnalysis/dependence/edgeList/private_el.h>


Boolean
el_get_view_lc (EL_Instance* el)
{
	return (el->lc);
}


Boolean
el_get_view_control (EL_Instance* el)
{
	return (el->control);
}


Boolean
el_get_view_li (EL_Instance* el)
{
	return (el->li);
}

Boolean
el_get_view_private (EL_Instance* el)
{
	return (el->privatev);
}


void 
el_set_view_lc (EL_Instance* el, Boolean lc)
{
	el->lc = lc;
}
void 
el_set_view_control (EL_Instance* el, Boolean control)
{
	el->control = control;
}
void 
el_set_view_li (EL_Instance* el, Boolean li)
{
	el->li = li;
}
void 
el_set_view_private (EL_Instance* el, Boolean privatev)
{
	el->privatev = privatev;
}
