/* $Id: f2i_options.h,v 1.5 1998/07/07 20:26:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef f2i_options_h
#define f2i_options_h

#include <libs/support/optParsing/Options.h>

#define F2I_PGM_OPT           'P'
#define F2I_MOD_OPT           'M'

#define F2I_ANNOTATE_FLAG     'a'
#define F2I_ALIGNDOUBLES_FLAG 'n'
#define F2I_CONSTANTS_FLAG    'C'
#define F2I_DEBUG_FLAG        'd'
#define F2I_ENREGGLOBALS_FLAG 'E'
#define F2I_FATALS_FLAG       'F'
#define F2I_GENERATE_FLAG     'g'
#define F2I_MESSAGEID_FLAG    'I'
#define F2I_NOALIAS_FLAG      'A'
#define F2I_PARSECOMMENTS_FLAG 'p'
#define F2I_SYMDUMP_FLAG      's'
#define F2I_TREECHECK_FLAG    'c'
#define F2I_TREEDUMP_FLAG     't'
#define F2I_VIRTUAL_FLAG      'v'
#define F2I_SYMMAP_FLAG       'm'
#define F2I_SPARC_FLAG        'S'
#define F2I_ROCKET_FLAG       'R'
#define F2I_RT_FLAG           'r'
#define F2I_CACHE_FLAG        'h'
#define F2I_SPECIALCACHE_FLAG 'H'
#define F2I_LONGINTEGERS_FLAG 'l'
#define F2I_DOUBLEREALS_FLAG  'D'
#define F2I_DEBUG_CHOICE      'X'
#define F2I_ADDRESSOPT_FLAG   'o'

EXTERN(int,f2i_init_options,(int,char**));
EXTERN(void,f2i_options_usage,(char *pgm_name));

#endif
