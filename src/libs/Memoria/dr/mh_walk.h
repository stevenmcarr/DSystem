/* $Id: mh_walk.h,v 1.13 1997/03/27 20:23:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef mh_walk_h
#define mh_walk_h

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>

#ifndef context_h
#include <libs/support/database/context.h>
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

#ifndef FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif

#ifndef FortTextTree_h
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif


typedef struct walkinfotype {
  int     selection;
  PedInfo ped;
  FortTree ft;
  FortTextTree ftt;
  SymDescriptor symtab;
  arena_type    *ar;
  LoopStatsType *LoopStats;
  char          *routine;
  char          *program;
  Boolean       MainProgram;
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

#endif
