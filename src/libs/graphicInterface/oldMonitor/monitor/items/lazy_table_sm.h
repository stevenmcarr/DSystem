/* $Id: lazy_table_sm.h,v 1.4 1997/03/11 14:33:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
EXTERN(short, sm_lazy_table_get_index,(void));
/*
 *
 */

EXTERN(Point, sm_lazy_table_pane_size,(Point size, short font));
/*	Point size;
 *	short font_id;
 */

EXTERN(void, sm_lazy_table_initialize,(Pane *p, short font_id));
/*      Pane    *p;
 *	short   font_id;
 */

EXTERN(void, sm_lazy_table_new_table,(Pane *p, Point nstrings, PFS string,
                                      Generic client, Point selected, 
                                      int item_width));
/*	Pane   *p;
 *	int     num_strings;
 *	char  **strings;
 *      int     selected;
 */

EXTERN(Point, sm_lazy_table_get_selection,(Pane *p));
/* Pane *p;
 */
 
EXTERN(void, sm_lazy_table_configuration,(Pane *p, int *width, int *height,
                                          Point *offset));
/* Pane *p,
 * int  *fullwidth;
 * int  *viewwidth;
 * int  *offset;
 */

		
EXTERN(void, sm_lazy_table_paint,(Pane *p));
/*
 * Pane	*p;
 */


EXTERN(void, sm_lazy_table_invalidate_item,(Pane *p, Point changed_loc));
/*
 * Pane	*p;
 * Point changed_item;
 */

EXTERN(void, sm_lazy_table_shift_absolute,(Pane *p, Point curval));
/*
   Pane	*p;
   int   newval;
 */


EXTERN(void, sm_lazy_table_shift_x_absolute,(Pane *p, int curval));
/*
   Pane	*p;
   int newval;
 */


EXTERN(void, sm_lazy_table_shift_y_absolute,(Pane *p, int curval));
/*
   Pane	*p;
   int   newval;
 */


EXTERN(void, sm_lazy_table_shift_relative,(Pane *p, Point shift));
/*
   Pane	*p;
   int   shift;
 */
