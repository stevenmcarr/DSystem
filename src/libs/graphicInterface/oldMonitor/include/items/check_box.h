/* $Id: check_box.h,v 1.3 1997/03/11 14:33:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				check_box.h				*/
/*			check box item include file			*/
/*									*/
/* Check boxes allow the user to toggle a flag.  The value is repre-	*/
/* sented as a Boolean with true being the selected position.		*/
/* 									*/
/************************************************************************/

#ifndef items_check_box_h
#define items_check_box_h

EXTERN(DiaDesc *, item_check_box, (Generic item_id, char *title,
 short font, Boolean *vptr));
/* check box item descriptor	*/
/* Takes four parameters:  (Generic item_id) the id of this item,	*/
/* (char *title) the title of this check box, (short font) the font	*/
/* of the check box, and (Boolean *vptr) the value which is modified	*/
/* by this check box.  A dialog descriptor of the item is returned.	*/

#endif
