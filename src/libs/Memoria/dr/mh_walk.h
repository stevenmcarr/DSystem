/* $Id: mh_walk.h,v 1.9 1993/09/06 14:54:15 carr Exp $ */

#ifndef mh_walk_h
#define mh_walk_h

#include <dp.h>

#ifndef context_h
#include <context.h>
#endif

#ifndef fortsym_h
#include <fort/fortsym.h>
#endif

#ifndef Arena_h
#include <misc/Arena.h>
#endif

#ifndef LoopStats_h
#include <LoopStats.h>
#endif

#ifndef FortTree_h
#include <FortTree.h>
#endif

#ifndef FortTextTree_h
#include <FortTextTree.h>
#endif

#ifndef ast_h
#include <ast.h>
#endif

#include <ArrayTable.h>

typedef struct walkinfotype {
  int     selection;
  PedInfo ped;
  FortTree ft;
  FortTextTree ftt;
  TableInfoType TableInfo;
  SymDescriptor symtab;
  arena_type    *ar;
  LoopStatsType LoopStats;
  char          *routine;
  char          *program;
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
