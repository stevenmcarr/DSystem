/* $Id: analyze.C,v 1.17 1997/03/27 20:29:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


/****************************************************************************/
/*                                                                          */
/*    File:   analyze.C                                                     */
/*                                                                          */
/*    Description:  Analyze a loop nest.  Create a tree to represent the    */
/*                  loop structure and store information for the loops in   */
/*                  nodes.                                                  */
/*                                                                          */
/****************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/Memoria/include/analyze.h>

#ifndef gi_h
#include <libs/frontEnd/include/gi.h>
#endif

#ifndef dg_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#endif

#ifndef dt_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#endif

#ifndef mh_config_h
#include <libs/Memoria/include/mh_config.h>
#endif

#include <libs/Memoria/include/mem_util.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <stdarg.h>
#include <libs/frontEnd/ast/treeutil.h>
#include <assert.h>

extern Boolean mc_allow_expansion;

/****************************************************************/
/*                                                              */
/*   Function:   CheckForEdgesNotHandled                        */
/*                                                              */
/*   Input:      stmt - statement in AST                        */
/*               level - nesting level of stmt                  */
/*               ped - structure containing dependence graph    */
/*                                                              */
/*   Description:  determines if io, control, exit and call     */
/*                 dependences inhibit interchange.             */
/*                                                              */
/****************************************************************/

static int CheckForEdgesNotHandled(AST_INDEX     stmt,
				   int           level,
				   CheckInfoType *CheckInfo)

  {
   DG_Edge    *dg;
   int        vector;
   EDGE_INDEX edge,
              next_edge;
   int        i;

     dg = dg_get_edge_structure( PED_DG(CheckInfo->ped));
     vector = get_info(CheckInfo->ped,stmt,type_levelv);
   
       /* remove carried dependences */

     for (i = 1;i <= level; i++)
       {

	        /* remove outgoing dependences */

	for (edge = dg_first_src_stmt( PED_DG(CheckInfo->ped),vector,i);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_stmt( PED_DG(CheckInfo->ped),edge);
	   switch(dg[edge].type)
	     {
	      case dg_exit:
	      case dg_io:
	      case dg_call:
	         CheckInfo->illegal = true;
		 break;
	      case dg_control:

		 /* do we have an DO-loop that is conditionally executed */

		 if (is_do(dg[edge].sink) &&
		     (dg[edge].cdtype == CD_LOGICAL_IF ||
		      dg[edge].cdtype == CD_ARITHMETIC_IF ||
		      dg[edge].cdtype == CD_COMPUTED_GOTO ||
		      dg[edge].cdtype == CD_ASSIGNED_GOTO))
	           CheckInfo->illegal = true;
		 break;
	      default:
		 break;
	      dg_delete_free_edge(PED_DG(CheckInfo->ped),edge);
	     }
	  }

	        /* remove incoming dependences */

	for (edge = dg_first_sink_stmt( PED_DG(CheckInfo->ped),vector,i);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {  
	   next_edge = dg_next_sink_stmt( PED_DG(CheckInfo->ped),edge);
	   switch(dg[edge].type)
	     {
	      case dg_exit:
	      case dg_io:
	      case dg_call:
	         CheckInfo->illegal = true;
		 break;
	      case dg_control:

		 /* do we have an DO-loop that is conditionally executed */

		 if (is_do(dg[edge].sink) &&
		     (dg[edge].cdtype == CD_LOGICAL_IF ||
		      dg[edge].cdtype == CD_ARITHMETIC_IF ||
		      dg[edge].cdtype == CD_COMPUTED_GOTO ||
		      dg[edge].cdtype == CD_ASSIGNED_GOTO))
	           CheckInfo->illegal = true;
		 break;
	      default:
		 break;	 
	      dg_delete_free_edge(PED_DG(CheckInfo->ped),edge);
	     }
	  }
       }

	        /* remove outgoing loop-independent dependences */

     for (edge = dg_first_src_stmt( PED_DG(CheckInfo->ped),vector,LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_src_stmt( PED_DG(CheckInfo->ped),edge);
	switch(dg[edge].type)
	  {
	   case dg_exit:
	   case dg_io:
	   case dg_call:
	     CheckInfo->illegal = true;
	     break;
	   case dg_control:

	     /* do we have an DO-loop that is conditionally executed */
	     
	     if (is_do(dg[edge].sink) &&
		 (dg[edge].cdtype == CD_LOGICAL_IF ||
		  dg[edge].cdtype == CD_ARITHMETIC_IF ||
		  dg[edge].cdtype == CD_COMPUTED_GOTO ||
		  dg[edge].cdtype == CD_ASSIGNED_GOTO))
	       CheckInfo->illegal = true;
	     break;
	   default:
	     break;
	   dg_delete_free_edge(PED_DG(CheckInfo->ped),edge);
	  }	
       }

	        /* remove incoming loop-independent dependences */

     for (edge = dg_first_sink_stmt( PED_DG(CheckInfo->ped),vector,LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_sink_stmt( PED_DG(CheckInfo->ped),edge);
	switch(dg[edge].type)
	  {
	   case dg_exit:
	   case dg_io:
	   case dg_call:
	     CheckInfo->illegal = true;
	     break;
	   case dg_control:

	     /* do we have an DO-loop that is conditionally executed */
	     
	     if (is_do(dg[edge].sink) &&
		 (dg[edge].cdtype == CD_LOGICAL_IF ||
		  dg[edge].cdtype == CD_ARITHMETIC_IF ||
		  dg[edge].cdtype == CD_COMPUTED_GOTO ||
		  dg[edge].cdtype == CD_ASSIGNED_GOTO))
	        CheckInfo->illegal = true;
	     break;
	   default:
	     break;
	   dg_delete_free_edge(PED_DG(CheckInfo->ped),edge);
	  }	
       }
     return(WALK_CONTINUE);
  }



/****************************************************************************/
/*                                                                          */
/*    Function:     build_pre                                               */
/*                                                                          */
/*    Input:        stmt - AST index of a statement                         */
/*                  level - nesting level of stmt                           */
/*                  build_info - structure containing loop information.     */
/*                                                                          */
/*    Description:  Create a loop structure entry for each do loop and      */
/*                  initialize the information.                             */
/*                                                                          */
/****************************************************************************/


static int build_pre(AST_INDEX       stmt, int             level,
		     build_info_type *build_info)
  {
   AST_INDEX upb,step,lval,lwb;
   int       loop_num,upb_v,step_v,lwb_v;
   CheckInfoType CheckInfo;

     if (is_do(stmt))
       {
	fst_PutField(build_info->symtab,
		     gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(stmt))),
		     IVAR,true);
	loop_num = get_stmt_info_ptr(stmt)->loop_num;
	build_info->loop_data[loop_num].inner_loop = -1;
	build_info->loop_data[loop_num].next_loop = -1;
	build_info->loop_data[loop_num].tri_loop = -1;
	build_info->loop_data[loop_num].trap_loop = -1;

	   /* link parent and child in loop structure */

	if (build_info->last_loop == 0)
	  if (build_info->parent != -1)
            build_info->loop_data[build_info->parent].inner_loop = loop_num;
	  else;
	else 
          build_info->loop_data[build_info->last_loop].next_loop = loop_num;
	build_info->loop_data[loop_num].parent = build_info->parent;  

	build_info->loop_data[loop_num].node = stmt;
	build_info->loop_data[loop_num].level = level;
	build_info->loop_data[loop_num].transform = true;
	build_info->loop_data[loop_num].reduction = false;
	build_info->loop_data[loop_num].reversed = false;
	build_info->loop_data[loop_num].unroll = false;
	build_info->loop_data[loop_num].interchange = true;
	build_info->loop_data[loop_num].distribute = true;
	build_info->loop_data[loop_num].expand = false;
	build_info->loop_data[loop_num].NoImprovement = false;
	build_info->loop_data[loop_num].Distribute = false;
	build_info->loop_data[loop_num].DistributeNumber = 0;
	build_info->loop_data[loop_num].Interchange = false;
	build_info->loop_data[loop_num].InterlockCausedUnroll = false;
	build_info->loop_data[loop_num].rho = 0;
	build_info->loop_data[loop_num].stride = 0;
	build_info->loop_data[loop_num].scalar_array_refs = 0;
	build_info->loop_data[loop_num].ibalance= 0;
	build_info->loop_data[loop_num].fbalance= 0;
	build_info->loop_data[loop_num].registers = 0;
	build_info->loop_data[loop_num].type = RECT;
	build_info->loop_data[loop_num].inner_stmts = MAXLOOP;
	build_info->loop_data[loop_num].outer_stmts = -1;
	build_info->loop_data[loop_num].CarriedDependences= 0;
	build_info->loop_data[loop_num].OutermostLvl = 0;
	build_info->loop_data[loop_num].GroupList = 
	                                util_list_alloc((Generic)NULL,NULL);
	build_info->loop_data[loop_num].OutermostLvl = 0;

	  /* determine maximum unroll amount based on loop iterations */

	step = gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(stmt));
	if (!pt_eval(step,&step_v) || step == AST_NIL)
	  {
	   upb = gen_INDUCTIVE_get_rvalue2(gen_DO_get_control(stmt));
	   lwb = gen_INDUCTIVE_get_rvalue1(gen_DO_get_control(stmt));
	   if (!pt_eval(upb,&upb_v) && !pt_eval(lwb,&lwb_v))
	     {
	      if (step == AST_NIL)
	        step_v = 1;
	      else
	        if (step_v < 0)
		  step_v = -step_v;
	      upb_v = (upb_v - lwb_v)/step_v;
	      if (upb_v < 0)
	        build_info->loop_data[loop_num].max = -upb_v;
	      else
	        build_info->loop_data[loop_num].max = upb_v;
	     }
	   else
	     build_info->loop_data[loop_num].max = 
	         ((config_type *)PED_MH_CONFIG(build_info->ped))->max_regs;
	  }
	else
	  build_info->loop_data[loop_num].max = 0;
	build_info->loop_data[loop_num].split_list = 
	          util_list_alloc((Generic)NULL,"split-list");
	build_info->last_stack[++build_info->stack_top] = loop_num;
	build_info->last_loop = 0;

	   /* new parent as we move in a level */

	CheckInfo.ped = build_info->ped;
	CheckInfo.illegal = false;
	walk_statements(gen_DO_get_stmt_LIST(stmt),level,(WK_STMT_CLBACK)
			(WK_STMT_CLBACK)CheckForEdgesNotHandled,(WK_STMT_CLBACK)NOFUNC,
			(Generic)&CheckInfo);
	if (CheckInfo.illegal)
	  {
	   build_info->loop_data[loop_num].DependencesHandled = false; 
	   build_info->loop_data[loop_num].distribute = false; 
	   build_info->loop_data[loop_num].transform = false;
	  }
	else
	  build_info->loop_data[loop_num].DependencesHandled = true; 
	build_info->parent = loop_num;
       }
     else
       {

	   /* this loop contains non-do statements */

	build_info->loop_data[build_info->last_stack[build_info->stack_top]].
                                                                stmts = true;
	if (is_assignment(stmt))
	  {
	   lval = gen_ASSIGNMENT_get_lvalue(stmt);
	   if (is_identifier(lval) && 
	       (gen_get_converted_type(lval) == TYPE_DOUBLE_PRECISION ||
	        gen_get_converted_type(lval) == TYPE_REAL))
	     build_info->loop_data[build_info->
			   last_stack[build_info->stack_top]].reduction
			   = pt_find_var(gen_ASSIGNMENT_get_rvalue(stmt),
					 gen_get_text(lval));
	  }
       }
     return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     build_post                                              */
/*                                                                          */
/*    Input:        stmt - AST index of a statement                         */
/*                  level - nesting level of stmt                           */
/*                  build_info - structure containing loop information.     */
/*                                                                          */
/*    Description:  If we are at a do, we have moved out one level.  Update */
/*                  parent pointer for this.                                */
/*                                                                          */
/****************************************************************************/


static int build_post(AST_INDEX       stmt,
		      int             level,
		      build_info_type *build_info)
 
  {
   if (is_do(stmt))
     {
      build_info->last_loop = build_info->last_stack[build_info->stack_top--];
      build_info->parent = build_info->loop_data[build_info->last_loop].parent;
     }
   return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     check_uj_preventing                                     */
/*                                                                          */
/*    Input:        loop_data - loops structure                             */
/*                  loop - index of loop to be checked                      */
/*                  edge - dependence edge to be checked                    */
/*                                                                          */
/*    Description:  Determine if edge prevents unroll-and-jam or limits it  */
/*                                                                          */
/****************************************************************************/


static void check_uj_preventing(model_loop *loop_data,
				int        loop,
				DG_Edge    edge)

  {
   Boolean illegal = false;
   int     dist,
           i;
   int     do1,do2;

     if (edge.dt_type == DT_UNKNOWN)
       {
	loop_data[loop].max = 0;
	loop_data[loop].interchange = false;
       }
     else if (edge.type != dg_input)
       {
	i = edge.level + 1;
	while(i <= gen_get_dt_LVL(&edge) && !illegal)
	  {
	   dist = gen_get_dt_DIS(&edge,i);
	   switch(dist) {
	     case DDATA_LE:
	     case DDATA_LT: break;

	     case DDATA_GT:
	     case DDATA_GE:
	     case DDATA_NE:  illegal = true;
	                     break;

	     case DDATA_ANY: if (edge.consistent != consistent_SIV || 
				 edge.symbolic)
	                       illegal = true;
	                     break;

	     case DDATA_ERROR: break;

	     default:
	       if (dist < 0)
	         illegal = true;
	    }
	   i++;
	  }
	if (illegal)
	  {
	   loop_data[loop].interchange = false;
	   dist = gen_get_dt_DIS(&edge,edge.level);
	   if (dist < DDATA_BASE)
             loop_data[loop].max = 0;
	   else
	     loop_data[loop].max = dist - 1;
	  }
       }
  }


static Boolean ParentIsSubscript(AST_INDEX node,
				 AST_INDEX stmt)

  {
   for (node = tree_out(node);
	node != stmt;
	node = tree_out(node))
     {
      assert(node != AST_NIL);
      if (is_subscript(node))
        return true;
     }
   return false;
  }

static void CheckIvar(AST_INDEX id,
		      int       atype,
		      va_list   args)

  {
   SymDescriptor symtab;
   Boolean *contains;
   AST_INDEX stmt;

     symtab = va_arg(args,SymDescriptor);
     contains = va_arg(args,Boolean*);
     stmt = va_arg(args,AST_INDEX);
     if (fst_GetField(symtab,gen_get_text(id),IVAR) &&
	 ParentIsSubscript(id,stmt))
       *contains = true;
  }

static Boolean containsIVAR(AST_INDEX stmt,
			    SymDescriptor symtab)

  {
   Boolean contains = false;

     walkIDsInStmt(stmt,(WK_IDS_CLBACK_V)CheckIvar,symtab,&contains,stmt);
     return(contains);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:      check_backwards_dep                                    */
/*                                                                          */
/*    Input:         loop - loop structure entry                            */
/*                   dg  - dependence graph                                 */
/*                   edge - dependence edge index                           */
/*                   ped - info handle                                      */
/*                   symtab - symbol table                                  */
/*                                                                          */
/*    Description:   Determine if an edge crosses loop boundaries and       */
/*                   prevents distribution.                                 */
/*                                                                          */
/****************************************************************************/


static void check_backwards_dep(model_loop *loop,
				DG_Edge    *dg,
				int        edge,
				PedInfo    ped,
				SymDescriptor symtab)

  {
   stmt_info_type *sptr1,*sptr2;

      sptr1 = get_stmt_info_ptr(ut_get_stmt(dg[edge].src));
      sptr2 = get_stmt_info_ptr(ut_get_stmt(dg[edge].sink));
      if (sptr1 != (Generic)NULL && sptr2 != (Generic)NULL)
        if ((sptr1->surrounding_do != sptr2->surrounding_do ||
                 /* special case where sink is in a do statement */
	    (is_do(ut_get_stmt(dg[edge].src)) &&
	     containsIVAR(ut_get_stmt(dg[edge].sink),symtab))) &&
	     sptr1->stmt_num > sptr2->stmt_num) 
	  {
	   loop->max = 0;
	   loop->distribute = false;
	   if (fst_GetField(symtab,gen_get_text(dg[edge].src),SYMTAB_NUM_DIMS)
	       > 0 || is_do(ut_get_stmt(dg[edge].src)) ||
	       is_do(ut_get_stmt(dg[edge].sink)))
	     loop->expand = false;
	  }
	else;
      else
        dg_delete_free_edge( PED_DG(ped),edge);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     crossing_loop                                           */
/*                                                                          */
/*    Input:        loop_data - loop structure                              */
/*                  loop1, loop2 - indices for loops                        */
/*                                                                          */
/*    Description:  Determine the innermost common surrounding loop for two */
/*                  loops within a nest                                     */
/*                                                                          */
/****************************************************************************/


static int crossing_loop(model_loop *loop_data,
			 int        loop1,
			 int        loop2)

  {
     while (loop1 != loop2)
       {
	if (loop_data[loop1].level < loop_data[loop2].level)
	  loop2 = loop_data[loop2].parent;
	else if (loop_data[loop2].level < loop_data[loop1].level)
	  loop1 = loop_data[loop1].parent;
	else
	  {
	   loop1 = loop_data[loop1].parent;
	   loop2 = loop_data[loop2].parent;
	  }
       }
     return(loop1);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     check_crossing_dep                                      */
/*                                                                          */
/*    Input:        loop_data - loop structure                              */
/*                  dg - dependence graph                                   */
/*                  edge - edge index                                       */
/*                  ped - various info                                      */
/*                  symtab - symbol table                                   */
/*                                                                          */
/*    Description:  Determine if scalar expansion is necessary.             */
/*                                                                          */
/****************************************************************************/


static void check_crossing_dep(model_loop *loop_data,
			       DG_Edge    *dg,
			       int        edge,
			       PedInfo    ped,
			       SymDescriptor symtab)

  {
   stmt_info_type *sptr1,*sptr2;
   int loop;

      if (fst_GetField(symtab,gen_get_text(dg[edge].src),SYMTAB_NUM_DIMS) != 0)
        return;
      sptr1 = get_stmt_info_ptr(ut_get_stmt(dg[edge].src));
      sptr2 = get_stmt_info_ptr(ut_get_stmt(dg[edge].sink));
      if (sptr1 != (Generic)NULL && sptr2 != (Generic)NULL)
        if (sptr1->surrounding_do != sptr2->surrounding_do ||
                 /* special case where sink is in a do statement */
	    (is_do(ut_get_stmt(dg[edge].sink)) && 
	     ut_get_stmt(dg[edge].sink) != ut_get_stmt(dg[edge].src)))
	  {
	   if (dg[edge].type == dg_true) 
	     {
	      loop = crossing_loop(loop_data,sptr1->surrounding_do,
				   sptr2->surrounding_do);
	      if (fst_GetField(symtab,gen_get_text(dg[edge].src),EXPAND_LVL) <
		  loop_data[loop].level)
	        fst_PutField(symtab,gen_get_text(dg[edge].src),EXPAND_LVL,
			     loop_data[loop].level);
	      if (loop_data[loop].distribute && mc_allow_expansion)
	        loop_data[loop].expand = true;
	      loop_data[loop].distribute = false;
	     }
	   dg_delete_free_edge( PED_DG(ped),edge);
	  }
	else;
      else
        dg_delete_free_edge( PED_DG(ped),edge);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:      build_edge_pre                                         */
/*                                                                          */
/*    Input:        stmt - AST index of a statement                         */
/*                  level - nesting level of stmt                           */
/*                  build_info - structure containing loop information.     */
/*                                                                          */
/*                                                                          */
/*    Description:  Walk the dependence graph and check the edges to see    */
/*                  if unroll-and-jam and distribution are legal            */
/*                                                                          */
/****************************************************************************/


static int build_edge_pre(AST_INDEX       stmt,
			  int             level,
			  build_info_type *build_info)

  {
   DG_Edge *dg;
   EDGE_INDEX edge,
              next_edge;
   int        vector,
              rec_num,
              thresh,
              max_thresh,
              lvl;
   float      rho_R;
   AST_INDEX  node;

   dg = dg_get_edge_structure( PED_DG(build_info->ped));
   vector = get_info(build_info->ped,stmt,type_levelv);
   for (lvl = build_info->level;
	lvl < level;
	lvl++)
     {
      rec_num = 0;
      max_thresh = 0;
      for (edge = dg_first_src_stmt( PED_DG(build_info->ped),vector,lvl);
	   edge != END_OF_LIST;
	   edge = next_edge)
	{
	 next_edge = dg_next_src_stmt( PED_DG(build_info->ped),edge);
	 if (dg[edge].type == dg_true || dg[edge].type == dg_anti ||
	     dg[edge].type == dg_output) 
	   if (fst_GetField(build_info->symtab,gen_get_text(dg[edge].src),
			  SYMTAB_NUM_DIMS) > 0)
	     {

	      /* is uj legal? */

	      check_uj_preventing(build_info->loop_data,
				  build_info->last_stack[lvl],dg[edge]);

	      /* compute interlock */
	      
	      if (stmt == ut_get_stmt(dg[edge].sink) && dg[edge].type == dg_true)
		{

		 /* this estimates the length and threshold of a single statement
		    recurrance.  Only single statement recurrances are used in
		    the computation of rho(L) */
	      
		 if (gen_is_dt_DIS(&dg[edge]))
	           thresh = gen_get_dt_DIS(&dg[edge],lvl);
		 else
	           thresh = 1;
		 rec_num = 1;
		 node = tree_out(tree_out(dg[edge].sink));
		 while(!stmt_containing_expr(tree_out(node)))
		   {
		    rec_num++;
		    node = tree_out(node);
		   }
		 rho_R = ((float) rec_num) / ((float) thresh);
		 if (build_info->loop_data[build_info->last_stack[lvl]].rho <
		     rho_R)
	           build_info->loop_data[build_info->last_stack[lvl]].rho =rho_R;
		}
	      else if (dg[edge].src == dg[edge].sink && 
		       dg[edge].consistent == consistent_SIV &&
		       !dg[edge].symbolic && dg[edge].type == dg_output)
	        build_info->loop_data[build_info->last_stack[lvl]].
	                                        scalar_array_refs++;
	      
	         /* is distribution legal? */
	      
	      check_backwards_dep(&build_info->loop_data[build_info->
	                          last_stack[lvl]],dg,edge,build_info->ped,
				  build_info->symtab);
	     }
	   else

	         /* is distribution legal? */

 	     check_backwards_dep(&build_info->loop_data[build_info->
	                         last_stack[lvl]],dg,edge,build_info->ped,
				 build_info->symtab);
	 else if (dg[edge].type == dg_inductive)
	   {
	    build_info->loop_data[build_info->last_stack[lvl]].max = 0;
	    build_info->loop_data[build_info->last_stack[lvl]].transform=false;
	   }
	}
     }

      /* check for scalar expansion */

   for (edge = dg_first_src_stmt( PED_DG(build_info->ped),vector,
				 LOOP_INDEPENDENT);
	edge != END_OF_LIST;
	edge = next_edge)
     {
      next_edge = dg_next_src_stmt( PED_DG(build_info->ped),edge);
      if (dg[edge].type == dg_true || dg[edge].type == dg_anti ||
	  dg[edge].type == dg_input || dg[edge].type == dg_output)
        check_crossing_dep(build_info->loop_data,dg,edge,build_info->ped,
			   build_info->symtab);
     }
   for (edge = dg_first_sink_stmt( PED_DG(build_info->ped),vector,
				  LOOP_INDEPENDENT);
	edge != END_OF_LIST;
	edge = next_edge)
     {
      next_edge = dg_next_sink_stmt( PED_DG(build_info->ped),edge);
      if (dg[edge].type == dg_true || dg[edge].type == dg_anti ||
	  dg[edge].type == dg_input || dg[edge].type == dg_output)
        check_crossing_dep(build_info->loop_data,dg,edge,build_info->ped,
			   build_info->symtab);
     }
   if (is_do(stmt))
     {
      build_info->parent = get_stmt_info_ptr(stmt)->loop_num;
      build_info->last_stack[build_info->loop_data[build_info->parent].level] 
        = build_info->parent;
     }
   return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     build_edge_post                                         */
/*                                                                          */
/*    Input:        stmt - AST index of a statement                         */
/*                  level - nesting level of stmt                           */
/*                  build_info - structure containing loop information.     */
/*                                                                          */
/*                                                                          */
/*    Description:  update the parent point because we go out a level       */
/*                                                                          */
/****************************************************************************/


static int build_edge_post(AST_INDEX       stmt,
			   int             level,
			   build_info_type *build_info)

  {
   if (is_do(stmt))
     build_info->parent = build_info->loop_data[build_info->parent].parent;
   return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     propogate_transform                                     */
/*                                                                          */
/*    Input:        loop_data - loop structure                              */
/*                  loop - loop index                                       */
/*                                                                          */
/*    Description:  If inner loops can't be transformed because of an       */
/*                  inductive dependence, than outer loops can't be either. */
/*                  Propogate this fact up the tree.                        */
/*                                                                          */
/****************************************************************************/


static Boolean propogate_transform(model_loop *loop_data,
				   int        loop)

  {
   Boolean can_transform;
   int     next;

     if (loop_data[loop].inner_loop != -1)
       loop_data[loop].transform = BOOL(loop_data[loop].transform &&
					propogate_transform(loop_data,
						  loop_data[loop].inner_loop));
     else
       loop_data[loop].max = 0;
     if (!loop_data[loop].transform)
       loop_data[loop].max = 0;
     can_transform = loop_data[loop].transform;
     for (next = loop_data[loop].next_loop;
	  next != -1;
	  next = loop_data[next].next_loop)
       {
       loop_data[loop].transform = BOOL(loop_data[loop].transform &&
				   propogate_transform(loop_data,next));
	if (!loop_data[loop].transform)
	  loop_data[loop].max = 0;
	can_transform = BOOL(can_transform && loop_data[next].transform);
       }
     return(can_transform);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:      ut_analyze_loop                                        */
/*                                                                          */
/*    Input:         root - AST index of a do-loop                          */
/*                   loop_data - loop structure                             */
/*                   level - nesting level of root                          */
/*                   ped - dependence graph handle                          */
/*                   symtab - symbol table                                  */
/*                                                                          */
/*    Description:   Create a tree structure representing a loop nest.      */
/*                   Loops at the same level are siblings, inner loops are  */
/*                   children.                                              */
/*                                                                          */
/****************************************************************************/



void ut_analyze_loop(AST_INDEX  root,
		     model_loop *loop_data,
		     int        level,
		     PedInfo    ped,
		     SymDescriptor symtab)

  {
   build_info_type build_info;
   AST_INDEX       upb;

     build_info.parent = -1;
     build_info.last_loop = 0;
     build_info.stack_top = level-1;
     build_info.level = level;
     build_info.loop_data = loop_data;
     build_info.ped = ped;
     build_info.symtab = symtab;
     build_info.last_stack[0] = 0;
     loop_data[0].parent = -1;
     fst_InitField(symtab,IVAR,false,0);

             /* initialize loop structure and link together entries */

     walk_statements(root,level,(WK_STMT_CLBACK)build_pre,(WK_STMT_CLBACK)build_post,(Generic)&build_info);
            
             /* examine dependence graph and check legality */

     walk_statements(root,level,(WK_STMT_CLBACK)build_edge_pre,(WK_STMT_CLBACK)build_edge_post,
		     (Generic)&build_info);
     if (loop_data[0].inner_loop != -1)
       loop_data[0].transform = propogate_transform(loop_data,
						    loop_data[0].inner_loop);
     else
       loop_data[0].transform = false;
     fst_KillField(symtab,IVAR);
  }
