/* $Id: function_sm.h,v 1.4 1997/03/11 14:33:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef function_sm_h
#define function_sm_h

# define FN_NORMAL	false
# define FN_ROTATED	true

EXTERN(short, sm_function_get_index, (void));

EXTERN(Point, sm_function_pane_size, (Point size));

typedef FUNCTION_POINTER(void, sm_function_select_callback,
 (Generic notify_handle, Point sel, Boolean move));

EXTERN(void, sm_function_setup, (Pane *p, int num_vals, double *value,
 Boolean rotate, Boolean move_zoomed,
 sm_function_select_callback select_notify, Generic notify_handle));

EXTERN(void, sm_function_new_values, (Pane *p, double *values,
 Boolean rescale));

EXTERN(void, sm_function_change_point, (Pane *p, int index));

EXTERN(void, sm_function_redraw, (Pane *p));

EXTERN(void, sm_function_clear_pane, (Pane *p));

EXTERN(Point, sm_function_get_selection, (Pane *p));
 
EXTERN(void, sm_function_set_selection, (Pane *p, Point sel));
 
EXTERN(int, sm_function_get_last_display, (Pane *p));
 
EXTERN(void, sm_function_set_last_display, (Pane *p, int last));
 
EXTERN(Point, sm_function_get_zoom_range, (Pane *p));
 
EXTERN(void, sm_function_set_zoom_range, (Pane *p, Point sel));
 
EXTERN(void, sm_function_get_min_max, (Pane *p, double *p_fmin, double *p_fmax));

EXTERN(void, sm_function_set_min_max, (Pane *p, double fmin, double fmax));
 
EXTERN(int, sm_function_get_draw_point_threshold, (Pane *p));

EXTERN(void, sm_function_set_draw_point_threshold, (Pane *p, int t));
 
EXTERN(Boolean, sm_function_get_rotate, (Pane *p));

EXTERN(void, sm_function_set_rotate, (Pane *p, Boolean r));
 
#endif
