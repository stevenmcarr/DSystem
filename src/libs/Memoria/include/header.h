#ifndef header_h
#define header_h

#include <general.h>
#include <database.h>
#include <ped.h>
#include <ast.h>
#include <strutil.h>
#include <astutil.h>
#include <astlist.h>
#include <astcons.h>
#include <astnode.h>
#include <aphelper.h>
#include <astsel.h>
#include <asttree.h>
#include <astrec.h>
/* #include "/rn/usr/johnmc/src/ned_cp/FortTree.h" */
#include <FortTree.h>
#include <TextTree.h>
#include <FortTextTree.h>
#include <fortsym.h>
#include <groups.h>
#include <cd.h>
#include <dg.h>
#include <el.h>
#include <dp.h>
#include <dt.h>
#include <Arena.h>

EXTERN_FUNCTION(void memory_loop_interchange,(PedInfo ped,AST_INDEX root,
					      int level,SymDescriptor symtab,
					      arena_type *ar));

EXTERN_FUNCTION(AST_INDEX memory_unroll_and_jam,(PedInfo ped,AST_INDEX root,
					     int level,int num_loops,
					     SymDescriptor symtab,
					     arena_type *ar));

EXTERN_FUNCTION(void memory_scalar_replacement,(PedInfo ped,AST_INDEX root,
						SymDescriptor symtab,
						arena_type *ar));

#endif
