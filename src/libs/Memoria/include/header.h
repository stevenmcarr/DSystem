/* $Id: header.h,v 1.4 1992/12/07 10:17:24 carr Exp $ */

#ifndef header_h
#define header_h

#include <general.h>
#include <newdatabase.h>
#include <ped.h>
#include <fort/ast.h>
#include <fort/strutil.h>
#include <fort/astutil.h>
#include <fort/astlist.h>
#include <fort/astcons.h>
#include <fort/astnode.h>
#include <fort/aphelper.h>
#include <fort/astsel.h>
#include <fort/asttree.h>
#include <fort/astrec.h>
/* #include "/rn/usr/johnmc/src/ned_cp/FortTree.h" */
#include <fort/FortTree.h>
#include <fort/TextTree.h>
#include <fort/FortTextTree.h>
#include <fort/fortsym.h>
#include <fort/groups.h>
#include <cd.h>
#include <dg.h>
#include <el.h>
#include <dp.h>
#include <dt.h>
#include <Arena.h>
#include <LoopStats.h>

EXTERN(void, memory_interchange_stats,(PedInfo ped,AST_INDEX root,
					       int level,
				               LoopStatsType *LoopStats,
					       SymDescriptor symtab,
					       arena_type *ar));

EXTERN(void, memory_loop_interchange,(PedInfo ped,AST_INDEX root,
					      int level,SymDescriptor symtab,
					      arena_type *ar));

EXTERN(AST_INDEX, memory_unroll_and_jam,(PedInfo ped,AST_INDEX root,
					     int level,int num_loops,
					     SymDescriptor symtab,
					     arena_type *ar));

EXTERN(void, memory_scalar_replacement,(PedInfo ped,AST_INDEX root,
						SymDescriptor symtab,
						arena_type *ar));

#endif
