/* $Id: header.h,v 1.16 1996/04/10 13:13:50 carr Exp $ */

#ifndef header_h
#define header_h

#ifndef general_h
#include <general.h>       /* for EXTERN */
#endif
#ifndef dp_h
#include <dp.h>                 /* for PedInfo */
#endif
#ifndef ast_h
#include <fort/ast.h>            /* for AST_INDEX */
#endif
#ifndef fortsym_h
#include <fort/fortsym.h>        /* for SymDescriptor */
#endif
#ifndef Arena_h
#include <misc/Arena.h>               /* for arena_type */
#endif
#ifndef LoopStats_h
#include <LoopStats.h>           /* for LoopStatsType */
#endif 

#include <fort/FortTextTree.h>
#include <ArrayTable.h>

EXTERN(void, memory_interchange_stats,(PedInfo ped,AST_INDEX root,
					       int level,
				               LoopStatsType *LoopStats,
				               char *routine,
				               char *program,
					       SymDescriptor symtab,
					       arena_type *ar));

EXTERN(void, memory_loop_interchange,(PedInfo ped,AST_INDEX root,
				      int level,SymDescriptor symtab,
				      arena_type *ar, Boolean Fusion));

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
#endif
