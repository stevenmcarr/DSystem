/* $Id: Reversal.C,v 1.2 1997/03/27 20:25:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************/
/*                                                              */
/*   File:   Reversal.C                                      */
/*                                                              */
/****************************************************************/

#ifndef general_h
#include <libs/support/misc/general.h>
#endif 

#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/Memoria/include/mh_config.h>
#include <libs/Memoria/li/MemoryOrder.h>
#include <libs/Memoria/li/Reversal.h>

#ifndef header_h
#include <libs/Memoria/include/header.h>
#endif

#ifndef dt_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#endif

#ifndef gi_h
#include <libs/frontEnd/include/gi.h>
#endif 

#ifndef stats_h
#include <libs/Memoria/li/stats.h>
#endif 

#ifndef analyze_h
#include <libs/Memoria/include/analyze.h>
#endif 

#ifndef shape_h
#include <libs/Memoria/include/shape.h>
#endif 

#ifndef mem_util_h
#include <libs/Memoria/include/mem_util.h>
#endif 

#ifndef mark_h
#include <libs/Memoria/include/mark.h>
#endif 

#ifndef LoopStats_h
#include <libs/Memoria/include/LoopStats.h>
#endif 

#ifndef dg_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#endif 
	      


/****************************************************************/
/*                                                              */
/*   Function:   ReversalMakesInterchangeLegal                  */
/*                                                              */
/****************************************************************/

static Boolean ReversalMakesInterchangeLegal(UtilList *EdgeList,
					     PedInfo  ped,
					     int      level)
  {
   UtilNode *Edge;
   DG_Edge  *dg;
   int      dist;
   EDGE_INDEX edge;

    
     dg = dg_get_edge_structure( PED_DG(ped));
     Edge = UTIL_HEAD(EdgeList);
     while(Edge != NULLNODE)
       {
	edge = UTIL_NODE_ATOM(Edge);
	dist = gen_get_dt_DIS(&dg[edge],level);

	switch(dist) {
	  case DDATA_GE:
	  case DDATA_GT: 
	  case DDATA_NE:  break;


	  case DDATA_ANY: 
	  case DDATA_LT:
	  case DDATA_LE:  return(false);
	                  break;

	  case DDATA_ERROR: return(false);
			    break;

	  default:
	          if (dist > 0)
		    return(false);
	       }
	Edge = UTIL_NEXT(Edge);
       }
     return(true);
  }


static void PerformLoopReversal(model_loop *loop_data,
				int        loop)

  {
   AST_INDEX from,to,step,control;

     control = gen_DO_get_control(loop_data[loop].node);
     from = gen_INDUCTIVE_get_rvalue1(control);
     to = gen_INDUCTIVE_get_rvalue2(control);
     step = gen_INDUCTIVE_get_rvalue3(control);
     gen_INDUCTIVE_put_rvalue1(control,AST_NIL);
     gen_INDUCTIVE_put_rvalue2(control,AST_NIL);
     gen_INDUCTIVE_put_rvalue3(control,AST_NIL);
     gen_INDUCTIVE_put_rvalue1(control,to);
     gen_INDUCTIVE_put_rvalue2(control,from);
     if (step == AST_NIL)
       gen_INDUCTIVE_put_rvalue3(control,pt_gen_int(-1));
     else
       gen_INDUCTIVE_put_rvalue3(control,
				 pt_simplify_expr(pt_gen_mul(pt_gen_int(-1),step)));
     loop_data[loop].reversed = true;
  }
     

/****************************************************************/
/*                                                              */
/*   Function:  li_ComputeMemoryOrder                           */
/*                                                              */
/****************************************************************/

Boolean li_LoopReversal(model_loop  *loop_data,
			int         loop,
			UtilList    *EdgeList,
			PedInfo     ped)

  {
   if (ReversalMakesInterchangeLegal(EdgeList,ped,loop_data[loop].level))
     {
      PerformLoopReversal(loop_data,loop);
      return(true);
     }
   else
     return(false);
  }
