/* $Id: header.h,v 1.21 1997/07/01 13:20:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef header_h
#define header_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif
#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif
#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
#endif
#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif
#ifndef LoopStats_h
#include <libs/Memoria/include/LoopStats.h>
#endif 

#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/Memoria/include/ArrayTable.h>

EXTERN(void, memory_interchange_stats,(PedInfo ped,AST_INDEX root,
					       int level,
				               LoopStatsType *LoopStats,
				               char *routine,
				               char *program,
					       SymDescriptor symtab,
					       arena_type *ar));

EXTERN(void, memory_loop_interchange,(PedInfo ped,AST_INDEX root,
				      int level,SymDescriptor symtab,
				      arena_type *ar));

EXTERN(AST_INDEX, memory_unroll_and_jam,(PedInfo ped,AST_INDEX root,
					 int level,int num_loops,
					 SymDescriptor symtab,
					 arena_type *ar,
					 LoopStatsType *LoopStats));

EXTERN(void, memory_scalar_replacement,(PedInfo ped,AST_INDEX root,
					int level,SymDescriptor symtab,
					arena_type *ar,
					LoopStatsType *LoopStats));

EXTERN(void, memory_software_prefetch,(PedInfo ped,AST_INDEX root,
				       int level,SymDescriptor symtab,
				       arena_type *ar));

EXTERN(void, memory_dead_cache_lines,(PedInfo ped,AST_INDEX root,
				       int level,SymDescriptor symtab,
				       arena_type *ar));

EXTERN(void, memory_AnnotateWithCacheCalls,(AST_INDEX root, int level,
					    char *routine,
					    FortTextTree ftt,
					    SymDescriptor symtab));

EXTERN (void, ApplyMemoryCompiler, (int selection, PedInfo ped, AST_INDEX root,
				    FortTree ft, Context contxt, 
				    char *mc_config_file));

EXTERN(void, memory_stats_total, (char *program));

EXTERN(void, memory_UnrollStatsTotal, (char *program));

EXTERN(void, memory_SRStatsTotal, (char *program));

EXTERN(void, SRStatsDump, (FILE *logfile, LoopStatsType *LoopStats));

EXTERN(void, memory_PerformCacheAnalysis, (PedInfo      ped,
					   SymDescriptor symtab,
					   arena_type   *ar,
					   AST_INDEX    root,
					   int          level));

EXTERN(void, memory_AnnotateWithLDSTCount, (AST_INDEX    root,
					    int          level,
					    Boolean      MainProgram));
EXTERN(void, memory_DepStatsTotal, (char *program));
EXTERN(void, memory_GetDependenceStats, (PedInfo      ped,
					 AST_INDEX    root,
					 LoopStatsType *LoopStats,
					 SymDescriptor symtab,
					 arena_type   *ar,
					 int          level));

EXTERN(void, DepStatsDump, (FILE *logfile, LoopStatsType *LoopStats));
#endif
