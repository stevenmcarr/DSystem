/* $Id: MemoriaOptions.h,v 1.4 1997/10/30 15:31:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef MemoriaOptions_h
#define MemoriaOptions_h

#include <libs/support/optParsing/Options.h>

#define MC_MOD_OPT           'M'
#define MC_PGM_OPT           'P'
#define MC_LST_OPT           'L'
#define MC_CFG_OPT           'C'
#define MC_OUT_OPT           'O'
#define MC_PARTITION_UNROLL_OPT           'w'

#define MC_RESTRICTED_FLAG   'R'
#define MC_ALL_FLAG           'a'
#define MC_ANNOTATE_FLAG      'c'
#define MC_EXTENDED_CACHE_FLAG 'e'
#define MC_INTERCHANGE_FLAG   'i'
#define MC_REPLACEMENT_CHOICE 'r'
#define MC_STATISTICS_FLAG    's'
#define MC_UJSTATS_FLAG       'j'
#define MC_UNROLL_FLAG        'u'
#define MC_UNROLL_CACHE_FLAG  'U'
#define MC_PREFETCH_FLAG      'p'
#define MC_DEPENDENCE_CHOICE  'd'
#define MC_CACHE_ANAL_FLAG    'h'
#define MC_LDST_ANAL_FLAG     'l'
#define MC_DEAD_FLAG          'D'
#define MC_DEPENDENCE_STATS_FLAG 'S'

EXTERN(int, MemoriaInitOptions,(int argc, char **argv));
EXTERN(void, MemoriaOptionsUsage,(char *pgm_name));

#endif
