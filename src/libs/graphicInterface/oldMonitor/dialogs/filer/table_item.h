/* $Id: table_item.h,v 1.4 1997/03/11 14:33:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*			        table_item.h				*/
/*			 table item include file			*/
/*									*/
/************************************************************************/

#ifndef table_item_h
#define table_item_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

EXTERN(DiaDesc*, item_table, (Generic item_id, short font, short *numentries,
                              char ***entries, short *selection, 
                              Boolean *redraw_table, Point size));	
/* table item descriptor */

#endif
