/* $Id: mh_walk.h,v 1.6 1992/12/16 11:34:51 carr Exp $ */

#ifndef mh_walk_h
#define mh_walk_h

#ifndef context_h
#include <context.h>
#endif

#ifndef fortsym_h
#include <fort/fortsym.h>
#endif

#ifndef Arena_h
#include <Arena.h>
#endif

#ifndef LoopStats_h
#include <LoopStats.h>
#endif

#ifndef ped_h
#include <ped.h>
#endif

#ifndef FortTree_h
#include <FortTree.h>
#endif

#ifndef ast_h
#include <ast.h>
#endif

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

EXTERN (void, ApplyMemoryCompiler, (int selection, PedInfo ped, AST_INDEX root,
				    FortTree ft, Context mod_context,
				    char *config_file));
EXTERN (void, memory_stats_total, ());

#endif
