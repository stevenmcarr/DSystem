/* $Id: cfg_utils.h,v 3.4 1997/03/11 14:35:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- cfg_utils.h
 *
 */
#ifndef cfg_utils_h
#define cfg_utils_h

EXTERN( fst_index_t, cfg_get_symid, (CfgInstance cfg, char *name) );
EXTERN( void, int_set, (int *array, int size, int value) );
EXTERN( void, short_set, (short *array, int size, int value) );
EXTERN( void, Boolean_set, (Boolean *array, int size, Boolean value) );

EXTERN(CfgList, cfg_list_push, (CfgList list, Generic item, 
                                char *who));
EXTERN(CfgList, cfg_list_pop, (CfgList list));
EXTERN(Generic, cfg_list_top, (CfgList list));
EXTERN(void, cfg_list_kill, (CfgList list));
EXTERN(CfgInstanceType, cfg_curr_inst_type, (CfgInstance cfg));

#endif /* !cfg_utils_h */
