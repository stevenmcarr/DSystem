/* $Id: integer.h,v 1.3 1997/03/11 14:33:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				 integer.h				*/
/*			  integer item include file			*/
/*									*/
/* Integers provide a way for the user to edit an integer value.	*/
/* 									*/
/************************************************************************/

#ifndef items_integer_h
#define items_integer_h

EXTERN(DiaDesc *, item_integer, (Generic item_id, char *title,
 short font, int *vptr));
/* integer value item descriptor*/
/* Takes four parameters:  (Generic item_id) the id of this item,	*/
/* (char *title) the integer name, (short font) the font to use, and	*/
/* (int *vptr) the integer value which is modified by this item.  A	*/
/* dialog descriptor of the item is returned.				*/

#endif
