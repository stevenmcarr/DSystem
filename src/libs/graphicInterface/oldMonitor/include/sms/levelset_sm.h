/* $Id: levelset_sm.h,v 1.5 1997/03/11 14:33:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef levelset_sm_h
#define levelset_sm_h

EXTERN(short, sm_levelset_get_index, (void));

EXTERN(Point, sm_levelset_pane_size, (Point a_size));

EXTERN(void, sm_levelset_new_values, (Pane *p, Point num_vals,
 double **value, Point top, Point btm, Boolean rescale));

EXTERN(void, sm_levelset_paint, (Pane *p));

EXTERN(void, sm_levelset_set_contours, (Pane *p, Boolean line, Boolean paint,
 Boolean field, Boolean Auto, double base, double step, Boolean auto_contrast));

EXTERN(void, sm_levelset_get_contours, (Pane *p, Boolean *line, Boolean *paint, 
 Boolean *field, Boolean *Auto, double *base, double *step, Boolean *auto_contrast));

EXTERN(void, sm_levelset_adjust_zoom, (Pane *p, Point display_origin, Point num_display, Point top, Point btm));

EXTERN(double, sm_levelset_get_max, (Pane *p));

EXTERN(void, sm_levelset_get_selection, (Pane *p, Point *p_tl, Point *p_br));

EXTERN(double, sm_levelset_get_min, (Pane *p));

#endif
