/* $Id: pick.h,v 1.5 1997/03/27 20:27:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef pick_h
#define pick_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef scalar_h
#include <libs/Memoria/sr/scalar.h>
#endif

#ifndef block_h
#include <libs/Memoria/include/block.h>
#endif

#ifndef cgen_set_h
#include <libs/Memoria/include/cgen_set.h>
#endif

#ifndef dg_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

typedef struct pickinfotype {
  block_type *block,
             *exit_block;
  Set        LI_avail,
             LI_rgen,
             LC_rgen_if_1,
             LI_pavail,
             LC_avail_if_1,
             LC_pavail_if_1;
  DG_Edge    *dg;
  Boolean    contains_cf;
  int        level;
  PedInfo    ped;
  Boolean    def;
  AST_INDEX  lhs;
 } pick_info_type;

EXTERN(void, sr_pick_possible_generators,(flow_graph_type flow_graph,
					  int level,
					  prelim_info_type *prelim_info,
					  PedInfo ped));

#endif
