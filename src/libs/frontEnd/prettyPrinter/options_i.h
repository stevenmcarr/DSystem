/* $Id: options_i.h,v 1.7 1997/03/11 14:30:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
# ifndef options_i_h
# define options_i_h


#include <libs/support/optParsing/Options.h>

void init_std_options(Options &opts);
void init_adifor_options(Options &opts);

#define FTX_EXPNS_REQD_IN_MAP_SIDE_ARRAY "full expr mapping required"

void ptree_options_usage(char *pgm_name);

/* external declarations of options externally visible */

void ftx_opt_dialect (void *state, Generic dialect_routine);
void ftx_opt_labelDO (void *state);
void ftx_opt_source (void *state);
# endif
