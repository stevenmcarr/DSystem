#ifndef a2i_options_h
#define a2i_options_h

#include <misc/Options.h>

#define A2I_PGM_OPT           'P'
#define A2I_MOD_OPT           'M'

#define A2I_ANNOTATE_FLAG     'a'
#define A2I_ALIGNDOUBLES_FLAG 'n'
#define A2I_CONSTANTS_FLAG    'C'
#define A2I_DEBUG_FLAG        'd'
#define A2I_ENREGGLOBALS_FLAG 'E'
#define A2I_FATALS_FLAG       'F'
#define A2I_GENERATE_FLAG     'g'
#define A2I_MESSAGEID_FLAG    'I'
#define A2I_NOALIAS_FLAG      'A'
#define A2I_PARSECOMMENTS_FLAG 'p'
#define A2I_SYMDUMP_FLAG      's'
#define A2I_TREECHECK_FLAG    'c'
#define A2I_TREEDUMP_FLAG     't'
#define A2I_VIRTUAL_FLAG      'v'
#define A2I_SYMMAP_FLAG       'm'
#define A2I_SPARC_FLAG        'S'
#define A2I_ROCKET_FLAG       'R'
#define A2I_RT_FLAG           'r'
#define A2I_CACHE_FLAG        'h'
#define A2I_LONGINTEGERS_FLAG 'l'
#define A2I_DOUBLEREALS_FLAG  'D'

void a2i_init_options(Options &opts);
void a2i_options_usage(char *pgm_name);

#endif
