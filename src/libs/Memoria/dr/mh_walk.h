/* $Id: mh_walk.h,v 1.4 1992/12/07 10:09:34 carr Exp $ */

#ifndef mh_walk_h
#define mh_walk_h

#include <context.h>
#include <newdatabase.h>
#include <fort/fortsym.h>
#include <Arena.h>
#include <LoopStats.h>

typedef struct walkinfotype {
  int     selection;
  PedInfo ped;
  FortTree ft;
  SymDescriptor symtab;
  arena_type    *ar;
  LoopStatsType LoopStats;
 } walk_info_type;

typedef struct decllisttype {
  AST_INDEX dbl_prec_list,
            real_list,
            cmplx_list;
 } decl_list_type;

#define REFD   "mh: refd"

EXTERN(void, mh_walk_ast,(int selection,PedInfo ped,AST_INDEX root,
			  FortTree ft, Context mod_context,
			  arena_type *ar));

EXTERN (void, mh_get_config, (int config));
EXTERN (void, ApplyMemoryCompiler, (int selection, PedInfo ped, AST_INDEX root,
				    FortTree ft, Context mod_context));
EXTERN (void, memory_stats_total, ());

#endif
