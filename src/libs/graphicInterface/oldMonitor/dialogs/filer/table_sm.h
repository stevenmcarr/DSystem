/* $Id: table_sm.h,v 1.4 1997/03/11 14:33:05 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef table_sm_h
#define table_sm_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

EXTERN (short, sm_table_get_index, (void));

EXTERN (void, oldtable_adjust, (Pane *p));

EXTERN (Point, sm_table_pane_size, (Point size, short font));

EXTERN (void, sm_table_initialize, (Pane *p, short font_id));

EXTERN (void, sm_table_new_table, (Pane *p, short nstrings, char **strings, 
                                   short selected));

EXTERN (void, sm_table_update_table, (Pane *p, short nstrings, char **strings,
                                      short selected));

EXTERN (int, table_map, (Pane *p, Point info));

EXTERN (short, sm_table_get_selection, (Pane *p));
 
EXTERN (void, sm_table_configuration, (Pane *p, short *fullwidth, short
                                       *viewwidth, short *offset));

EXTERN (void, sm_table_shift_absolute, (Pane *p, short curval));

EXTERN (void, sm_table_shift_relative, (Pane *p, short shift));

#endif
