/* $Id: dt.C,v 1.1 1997/06/25 15:08:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*-----------------------------------------------------------------------

	dt.c		Dependence Test Module
	
*/


#include <stdlib.h>
#include <string.h>

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>

#include <libs/frontEnd/ast/treeutil.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/support/strings/rn_string.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>
					/* and check prototypes		*/

#include <libs/moduleAnalysis/cfg/cfg.h>
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>
#include <libs/moduleAnalysis/valNum/val.h>

#ifdef SOLARIS
EXTERN(void,bzero,(char *s, size_t n));
#endif



struct LooprefParams_struct;
typedef struct LooprefParams_struct LooprefParams;

/*-------------------*/ 
/* global functions  */

void      convert_to_Expr(AST_INDEX node, Expr *expr, 
                          Loop_list *loop_nest);
/*-----------------*/ 
/* local functions */

STATIC(void,	   dt_walk_test,(DG_Instance *dg, DT_info *dt, 
                                 SideInfo *infoPtr, AST_INDEX node,
                                 DG_Edge *Earray, CfgInfo cfgModule,
                                 CfgInstance cfgInstance));
STATIC(void,	   dt_edge_test,(DG_Instance *dg, DT_info *dt, 
                                 SideInfo *infoPtr, DG_Edge *Earray,
                                 EDGE_INDEX Eindex, int level,
                                 CfgInstance cfgInstance));
STATIC(void,       dt_dep_all,(DT_info *dt, DG_Edge *Edge));
STATIC(Loop_list,  *dt_loop,(AST_INDEX node, DT_info *dt, 
                             SideInfo *infoPtr, 
                             LooprefParams *looprefParams));
STATIC(int,        dt_common_loops,(Loop_list *l1, Loop_list *l2));	
STATIC(Boolean,    dt_step,(Loop_data *loops));
STATIC(Boolean,    dt_ivar,(char *ivar, int coeff, Loop_list *ldata,
                            Subs_data *sdata, Boolean plus));
STATIC(int,        dt_clear_loopref,(AST_INDEX node, 
                                     SideInfo *infoPtr));
STATIC(int,        dt_init_loopref,(AST_INDEX stmt, int lvl, 
                                    LooprefParams *looprefParams));
STATIC(int,        dt_done_loopref,(AST_INDEX stmt, int lvl, 
                                    LooprefParams *looprefParams));
STATIC(int,        dt_init_ref,(AST_INDEX node, 
                                LooprefParams *looprefParams));
STATIC(Subs_list,  *dt_slist,(DT_info *dt, Loop_list *ldata, 
                              AST_INDEX subs, 
                              CfgInstance cfgInstance));
STATIC(Boolean,    dt_check_edge,(DG_Edge *Edge));
STATIC(Boolean,    is_linear,(Loop_list *loop_nest, AST_INDEX node, 
                              Boolean *is_ivar_only));


struct LooprefParams_struct
{
  	DT_info         *dt;            /* dependence testing info   */
  	SideInfo	*infoPtr;	/* side array access         */
	CfgInfo          cfgModule;     /* handle for global cfg     */
  	CfgInstance      cfgInstance;   /* handle for function's cfg */
};

/*------------------*/
/* global functions */

/*-----------------------------------------------------------------------

	dt_init()		Initialize DT data structs

*/
DT_info	*
dt_init(AST_INDEX root, SideInfo *infoPtr, CfgInfo cfgModule)
{
  DT_info	*dt;
  DG_Edge	*Earray;        /* array of all DG edges */

  dt = dt_create_info( );	/* create & init DT data structs */

  /*-------------------------------*/
  /* collect loop & reference info */
  {
    LooprefParams	 looprefParams;
	  
    looprefParams.dt	    = dt;
    looprefParams.infoPtr   = infoPtr;
    looprefParams.cfgModule = cfgModule;
    looprefParams.cfgInstance = NULL;
	  
    walk_statements(root, LEVEL1, (WK_STMT_CLBACK)dt_init_loopref, (WK_STMT_CLBACK)dt_done_loopref, 
		    (Generic)&looprefParams);
  }
  return  dt;
}


/*-----------------------------------------------------------------------

  	dt_update()		Update dependence vectors

	Assumes that the dependence edges are in place
	Updates local dependence testing info for edges

*/
void
dt_update(DG_Instance *dg, DT_info *dt, SideInfo *infoPtr, AST_INDEX root)
{
  /* first update loop & ref info */
  dt_update_loopref(dt, infoPtr, root);

  /* then recalculate dep test info in area	*/
  dt_walk_test(dg, dt, infoPtr, root, dg_get_edge_structure(dg), NULL, NULL);
}


/*-----------------------------------------------------------------------

	dt_update_tests()		Retest each DG edge.

	Assumes that the dependence edges are in place
	Updates local dependence testing info for edges. mpal 910622

*/
void
dt_update_tests(DG_Instance *dg, DT_info *dt, SideInfo *infoPtr, 
                AST_INDEX root, CfgInfo cfgModule)
{
  /* recalculate dep test info in area	*/
  dt_walk_test(dg, dt, infoPtr, root, dg_get_edge_structure(dg), cfgModule,NULL);
}


/*-----------------------------------------------------------------------

	dt_update_loopref()		Clear & update loop & ref info

*/
void
dt_update_loopref(DT_info *dt, SideInfo *infoPtr, AST_INDEX root)
{
  /*-------------------------------------*/
  /* first clear loop & ref info in area */
  
  walk_expression(root, (WK_EXPR_CLBACK)dt_clear_loopref, NULL, (Generic)infoPtr );

  /*-------------------------------*/
  /* collect loop & reference info */
  {
    LooprefParams	looprefParams;
    
    looprefParams.dt	      = dt;
    looprefParams.infoPtr     = infoPtr;
    looprefParams.cfgModule   = NULL;
    looprefParams.cfgInstance = NULL;
    
    walk_statements(root, LEVEL1, (WK_STMT_CLBACK)dt_init_loopref, (WK_STMT_CLBACK)dt_done_loopref,
		    (Generic)&looprefParams);
  }
} 


/*-----------------------------------------------------------------------

	dt_free()		Free dependence vectors

*/
void
dt_free(DT_info *dt)
{
  dt_finalize_info( dt );
}


/*-----------------------------------------------------------------------

	dt_ast_sub()	Find ast subscript node associated with argument.

	Returns 	`Original node' if a subscript node,
			else `enclosing node' if a subscript node.
	Failure:	returns original node.

*/
AST_INDEX 
dt_ast_sub(AST_INDEX node)
{
  AST_INDEX sub_node;

  if( is_subscript(node) )	  
    {
      return node;
    }
  else
    {
      sub_node = tree_out(node);
      return is_subscript(sub_node) ? sub_node : node;
    }
}


/*-----------------------------------------------------------------------

	dt_ast_stmt()		Get ast for statement

	Returns 	root of AST for statement containing node

*/
AST_INDEX 
dt_ast_stmt(AST_INDEX node)
{
  while (!is_statement(node))
    node = tree_out(node);

  return node;
}



/*-----------------------------------------------------------------------

	dt_ast_loop()		Get ast for loop enclosing node

	Returns 	root of AST for loop containing node

*/
AST_INDEX dt_ast_loop(AST_INDEX node)
{
  node = tree_out(node);

  while ((node != AST_NIL) && NOT(is_loop(node)))
    node = tree_out(node);
  return node;
}


/*-----------------------------------------------------------------------

	dt_test()		Apply dependence test to 2 refs

	Sets Edge->dt_type to:
				DT_UNKNOWN	if type of dependence not known
				DT_NONE		if no dependence exists

*/
void
dt_test(DT_info *dt, SideInfo *infoPtr, CfgInstance cfgInst, DG_Edge *Edge, 
        AST_INDEX loop1, AST_INDEX loop2, Boolean imprecise)
{
  AST_INDEX  src;
  AST_INDEX  sink;
  AST_INDEX  node;
  Loop_list *src_loop;
  Loop_list *sink_loop;
  Subs_list *src_sub;
  Subs_list *sink_sub;
  int        level;
  int        clevel;

  src = Edge->src;
  sink = Edge->sink;

  /*-----------------------------------------------------------*/
  /* if imprecise, assume dep exist on all levels & directions */

  if (imprecise)
    {
      /*---------------------------------------------*/
      /* get info about loops surrounding src & sink */

      src_loop  = (Loop_list *) dg_get_info(infoPtr, loop1, type_ref);
      sink_loop = (Loop_list *) dg_get_info(infoPtr, loop2, type_ref);

      if ((src_loop == NO_REF) || (sink_loop == NO_REF))
	{
	  /* die_with_message("dt_test(): missing REF info for loop"); */

	  Edge->dt_type = DT_UNKNOWN;
	  return;
	}

      /*------------------------------------*/
      /* calculate max & common loop levels */

      level  = MAX(src_loop->level, sink_loop->level);
      clevel = dt_common_loops(src_loop, sink_loop); 

      /*---------------------------*/
      /* build DT info for DG Edge */

      Edge->dt_type = DT_NONLIN | DT_NOPROVE | level | clevel << DT_LBITS;
      dt_dep_all(dt, Edge);
      return;
    }

  /*--------------------------------------------------------*/
  /* if scalar, assume dep exist on all levels & directions */

  if ((src == dt_ast_sub(src)) || (sink == dt_ast_sub(sink)))
    {
      /*---------------------------------------------*/
      /* get info about loops surrounding src & sink */

      src_loop  = (Loop_list *) dg_get_info(infoPtr, loop1, type_ref);
      sink_loop = (Loop_list *) dg_get_info(infoPtr, loop2, type_ref);

      if ((src_loop == NO_REF) || (sink_loop == NO_REF))
	{
	  /* die_with_message("dt_test(): missing REF info for loop"); */

	  Edge->dt_type = DT_UNKNOWN;
	  return;
	}

      /*------------------------------------*/
      /* calculate max & common loop levels */

      level  = MAX(src_loop->level, sink_loop->level);
      clevel = dt_common_loops(src_loop, sink_loop); 

      /*---------------------------*/
      /* build DT info for DG Edge */

      Edge->dt_type = DT_SCALAR | DT_NOPROVE | level | clevel << DT_LBITS;
      dt_dep_all(dt, Edge);
      return;
    }

  /*------------------------------------------------------*/
  /* must be array reference, apply dependence tests		*/

  src_sub  = (Subs_list *) dg_get_info( infoPtr, dt_ast_sub(src), type_ref);
  sink_sub = (Subs_list *) dg_get_info( infoPtr, dt_ast_sub(sink), type_ref);

  if ((src_sub == NO_REF) || (sink_sub == NO_REF))
    {
      /* Missing REF info for subscript */
      Edge->dt_type = DT_UNKNOWN;
      return;
    }

  src_loop  = src_sub->loop_nest;
  sink_loop = sink_sub->loop_nest;

  /*------------------------------------*/
  /* calculate max & common loop levels */

  level  = MAX(src_loop->level, sink_loop->level);
  clevel = dt_common_loops(src_loop, sink_loop); 

  dt_analyze(dt, Edge, cfgInst, src_sub, sink_sub, 
	     src_loop, sink_loop, clevel, true);

  /* If some level-dependent-constants are found, */
  /* fold them into summary and try again.	  */
  /* Merge the two results for more precision.	  */
  /* May remove some dependences on inner loops	  */

  if (Edge->dt_type  == DT_NONLIN) /* symbolics found */
    {
      Edge->dt_type = DT_NONLIN | DT_NOPROVE | level | clevel << DT_LBITS;
      dt_dep_all( dt, Edge);
    }
  else if (Edge->dt_type != DT_NONE) /* if not indep */
    {
      /* no dep if only loop indep dep && single stmt output dep */
      if (gen_is_dt_ALL_EQ(Edge) && (src == sink))
	Edge->dt_type = DT_NONE;
    }
}


/*-----------------------------------------------------------------------

	dt_set_intchg()		Set interchange bits on DG edges

	A dependence is Interchange Preventing if it changes
	from a forward to a backward dependence after  	
	loop interchange, or vice versa					

	A dependence is Interchange Sensitive if its level changes 
	after loop interchange (i.e., remains on interchanged loop)

	Also sets consistent flag

*/
void
dt_set_intchg(DG_Edge *Eptr, int level)
{
  int distance;

  /*--------------------------------------*/
  /* just give up if not known dep type	*/

  if (Eptr->type == dg_call || Eptr->type == dg_exit || Eptr->type == dg_io)
    return;

  /*--------------------------------------------------*/
  /* depedence is consistent if distance vector found */
  /* else assume not consistent for now		      */

  /*-----------------------------------------------------*/
  /* if loop independent OR at/past common nesting level */

  if ((level == LOOP_INDEPENDENT) || (level >= gen_get_dt_CLVL(Eptr)))
    {
      Eptr->ic_preventing = false; /* (=,=)	*/
      Eptr->ic_sensitive = false;
      return;
    }

  distance = gen_get_dt_DIS(Eptr, level+1);

  /*--------------------------------------------*/
  /* have direction vector element		*/

  if (gen_is_dt_DIR(distance))
    {
      switch (distance)
	{
	case DDATA_LT:		/* (<,<)	*/
	case DDATA_LE:		/* (<,<=)	*/
	  Eptr->ic_preventing = false;
	  Eptr->ic_sensitive = false;
	  break;

	case DDATA_GT:		/* (<,>)	*/
	case DDATA_GE:		/* (<,>=)	*/
	case DDATA_NE:		/* (<,<>)	*/
	case DDATA_ANY:		/* (<,*)	*/
	  Eptr->ic_preventing = true;
	  Eptr->ic_sensitive = false;
	  break;

	case DDATA_ERROR:
	default:
	  Eptr->ic_preventing = true; /* conservative */
	  Eptr->ic_sensitive = false;
	  break;
	}
    }

  /*----------------------------------------------*/
  /* have distance vector, calc flag bits exactly */

  else
    {
      if (!distance)		/* (<,=) */
	{
	  Eptr->ic_preventing = false;
	  Eptr->ic_sensitive = true;
	}
      else if (distance < 0)	/* (<,>) */
	{
	  Eptr->ic_preventing = true;
	  Eptr->ic_sensitive = false;
	}
      else			/* (<,<) */
	{
	  Eptr->ic_preventing = false;
	  Eptr->ic_sensitive = false;
	}
    }
}


/*-----------------------------------------------------------------------

	dt_sub()	Analyze and convert one dimension of subscript

	Expect most expressions normalized to form "2*A-4*C+5-..."
	Does not allow form "A-(B+C)", only "A-B-C"

	Use recursion to chase down multi-term expressions

*/
Boolean
dt_sub(AST_INDEX node, Subs_data *sdata, Loop_list *ldata, Boolean plus)
     /* AST_INDEX node;			subscript expr	*/
     /* Subs_data *sdata;		subscript data	*/
     /* Loop_list *ldata;		loop data      	*/
     /* Boolean plus;			sign of data	*/
{
  char *ivar;
  AST_INDEX term1;
  AST_INDEX term2;
  int constant, coeff;

  /*---------------------------------------------------*/
  /* unary minus (loop, then fall through) */

  while (is_unary_minus(node))
    {
      node = gen_UNARY_MINUS_get_rvalue(node);
      plus = NOT(plus);
    }

  /*---------------------------------------------------*/
  /* constants */

  if (is_constant(node))
    {
      constant = atoi(gen_get_text(node));
      sdata->constant += plus ? constant : -constant;
    }

  /*---------------------------------------------------*/
  /* id only */

  else if (is_identifier(node))
    {
      if (NOT(ast_eval(node, &constant))) 
	{
	  /* id actually has const value */
	  sdata->constant += plus ? constant : -constant;
	}
      else
	{
	  /* try to match with an index variable */
	  ivar = gen_get_text(node);
	  if (dt_ivar(ivar, 1, ldata, sdata, plus))
	    return true;
	}
    }

  /*---------------------------------------------------*/
  /* plus	or minus */

  else if (is_binary_plus(node))
    {
      term1 = gen_BINARY_PLUS_get_rvalue1(node);
      if (dt_sub(term1, sdata, ldata, plus))
	return true;

      term2 = gen_BINARY_PLUS_get_rvalue2(node);
      return dt_sub(term2, sdata, ldata, true);	/* always add	*/
    }

  /*---------------------------------------------------*/

  else if (is_binary_minus(node))
    {
      term1 = gen_BINARY_MINUS_get_rvalue1(node);
      if (dt_sub(term1, sdata, ldata, plus))
	return true;

      term2 = gen_BINARY_MINUS_get_rvalue2(node);
      return dt_sub(term2, sdata, ldata, false); /* always minus	*/
    }

  /*---------------------------------------------------*/
  /* multiply	*/

  else if (is_binary_times(node))
    {
      term1 = gen_BINARY_TIMES_get_rvalue1(node);
      term2 = gen_BINARY_TIMES_get_rvalue2(node);

      if (NOT(ast_eval(term1, &coeff)))	/* 1st term has const val */
	{
	  if (NOT(ast_eval(term2, &constant))) /* 2nd term has const val */
	    {
	      /* both terms constant */
	      constant *= coeff;
	      sdata->constant += plus ? constant : -constant;
	    }
	  else if (is_identifier(term2)) /* 2nd term is id */
	    {
	      ivar = gen_get_text(term2);
	      if (dt_ivar(ivar, coeff, ldata, sdata, plus))
		return true;
	    }
	  else
	    {
	      return true;	/* no idea what 2nd term is, give up	*/
	    }
	}
      else if (NOT(ast_eval(term2, &coeff))) /* 2nd term has const val */
	{
	  if (!is_identifier(term1)) 
	    return true;	/* no idea what 1st term is, give up	*/

	  ivar = gen_get_text(term1);
	  if (dt_ivar(ivar, coeff, ldata, sdata, plus))
	    return true;
	}
      else
	{
	  return true;		/* neither term const, give up	*/
	}
    }

  /*---------------------------------------------------*/
  /* not in proper normalized form	*/

  else
    return true;

  return false;			/* found linear subscript	*/
}				/* end_dt_sub */



/* ==================================================================== */
/*				local functions				*/
/* ==================================================================== */


/**********************/
/* Tree Walk Routines *
/**********************/

/*-----------------------------------------------------------------------

	dt_init_loopref()	Collect loop & reference info, put in info array

	Returns:		Code for walk_statements()

	Pre action performed by walk_statements()

*/
static int
dt_init_loopref(AST_INDEX stmt, int lvl, LooprefParams *looprefParams)
{
  AST_INDEX	 expr1;
  AST_INDEX	 expr2;
  DT_info	*dt      = looprefParams->dt; /* dependence testing info */
  SideInfo	*infoPtr = looprefParams->infoPtr; /* Side array access	 */

  /*-----------------------------------------------*/
  /* Check each subprogram in Module for symbolics */ 
	
  if (looprefParams->cfgModule &&
      (is_subroutine(stmt) || is_function(stmt) || is_program(stmt)))
    {
      looprefParams->cfgInstance = 
	cfg_build_inst(looprefParams->cfgModule, stmt, AST_NIL);
    }

  /*---------------------------------------*/
  /* if loop, get & store loop information */

  if (is_loop(stmt))
    {
      dg_put_info(infoPtr, stmt, type_ref, 
		  (Generic)dt_loop(stmt, dt, infoPtr, looprefParams));
    }

  /*---------------------------------------*/
  /* function call, skip if in loop nest   */
  /* no interprocedural analysis as of yet */

  else if ((lvl > 1) && is_call(stmt))
    {
      expr1 = gen_CALL_get_invocation(stmt);
      expr1 = gen_INVOCATION_get_actual_arg_LIST(expr1);
      walk_expression( expr1, (WK_EXPR_CLBACK)dt_init_ref, NULL, (Generic)looprefParams );
    }

  /*------------------------------------------------------------*/
  /* skip compound stmts, will be walking over components later */

  else if (is_compound(stmt))	
    {
      if (is_guard(stmt))
	{
	  walk_expression(gen_GUARD_get_rvalue(stmt), 
			  (WK_EXPR_CLBACK)dt_init_ref, NULL, (Generic)looprefParams);
	}
      else if (is_logical_if(stmt))
	{
	  walk_expression(gen_LOGICAL_IF_get_rvalue(stmt), 
			  (WK_EXPR_CLBACK)dt_init_ref, NULL, (Generic)looprefParams);
	}
    }

  /*-----------------------------------------------------------*/
  /* generic single statement, get & walk over component exprs */

  else
    {
      if (get_expressions(stmt, &expr1, &expr2) == UNKNOWN_STATEMENT)
	return WALK_CONTINUE; 

      if (expr1 != AST_NIL)
	walk_expression(expr1, (WK_EXPR_CLBACK)dt_init_ref, NULL, (Generic)looprefParams);

      if (expr2 != AST_NIL)
	walk_expression(expr2, (WK_EXPR_CLBACK)dt_init_ref, NULL, (Generic)looprefParams);
    }

  return WALK_CONTINUE;
}


/*-----------------------------------------------------------------------

	dt_done_loopref()	Record summary info from walk over tree.

	Returns:		Code for walk_statements()

	Post action performed by walk_statements()

*/

static int
dt_done_loopref(AST_INDEX stmt, int lvl, LooprefParams *looprefParams)
{
  /*-------------------------------------*/
  /* Store each subprogram's cfgInstance */
	
  if (is_subroutine(stmt) || is_function(stmt) || is_program(stmt)) 
    {
      dg_put_info(looprefParams->infoPtr, stmt, type_cfg_inst, 
		  (Generic)looprefParams->cfgInstance);
    }

  return WALK_CONTINUE;
}




/*-----------------------------------------------------------------------

	dt_walk_test()		Walk over AST, find all DG edges, test subs

*/
static void
dt_walk_test(DG_Instance *dg, DT_info *dt, SideInfo *infoPtr, AST_INDEX node, 
             DG_Edge *Earray, CfgInfo cfgModule, CfgInstance cfgInstance)
     /* AST_INDEX	 	 node;		must be stmt	*/
     /* DG_Edge		*Earray;		array of all DG edges */
{
  EDGE_INDEX	idx_old;	/* index of current dependence edge	*/
  EDGE_INDEX	idx_new;	/* index of next dependence edge	*/
  int		Lvec; 		/* level vector				*/
  int		level; 		/* level of dependence edge		*/
  int		nest; 		/* deepest nesting level		*/
  char         *name;           /* subprogram name used to get cfgInst  */
  CfgInstance   new_cfg_inst = cfgInstance;
  
  if (cfgModule &&
      (is_subroutine(node) || is_function(node) || is_program(node)))
    {
      name = gen_get_text(get_name_in_entry(node));
      new_cfg_inst = cfg_get_inst(cfgModule, name);
    }
       
  /*-------------------------------------------------------*/
  /* get level vector for stmt, use lvec to mark all edges */

  /* level = -1 == level independent  */
  /* level = 0  == scalar dependences */

  if ((Lvec = dg_get_info(infoPtr, node, type_levelv)) != UNUSED)
    {
      nest = dg_length_level_vector(dg, Lvec);

      for (level = LOOP_INDEPENDENT; level <= nest ; level++)
	{
	  idx_old = dg_first_src_stmt(dg, Lvec, level);
	  if (idx_old != UNUSED)
	    {
	      /* make sure we get the successor BEFORE we check edge	  */
	      /* since if check causes edge to be deleted, we can't go on */

	      while ((idx_new = dg_next_src_stmt(dg, idx_old)) != UNUSED)
		{
		  dt_edge_test(dg,dt,infoPtr,Earray,idx_old,level,new_cfg_inst);

		  idx_old = idx_new; /* now go to successor */
		}

	      dt_edge_test(dg, dt, infoPtr, Earray, idx_old, level,new_cfg_inst);
	    }
	}
    }

  /*-----------------------------------------------------------*/
  /* if compound node then continue on substmts, else finished */

  if (is_compound(node))
    {
      node = gen_get_stmt_list(node);
      node = list_first(node);
      while (node != AST_NIL)
	{
	  dt_walk_test(dg, dt, infoPtr, node, Earray, cfgModule, new_cfg_inst);
	  node = list_next(node);
	}
    }
}


/*-----------------------------------------------------------------------

	dt_init_ref()	Collect reference info, put in info array

	Returns:	Code for walk_expression()

	Pre action performed by walk_expression()

*/
static int
dt_init_ref(AST_INDEX node, LooprefParams *looprefParams)
{
  AST_INDEX   loop;
  Subs_list  *slist;
  Loop_list  *ldata;
  DT_info    *dt	  = looprefParams->dt; /* dependence testing info */
  SideInfo   *infoPtr     = looprefParams->infoPtr; /* Side array access  */
  CfgInstance cfgInstance = looprefParams->cfgInstance; /* cfg instance   */
  /*------------------------------------------*/
  /* if not identifier, do nothing			*/

  if (!is_subscript(node))
    return WALK_CONTINUE;	

  /*----------------------------------------------------------------*/
  /* find nearest enclosing loop surrounding subscripted reference, */
  /* then calculate & store info about subscripted reference        */

  loop = dt_ast_loop(node);

  if (loop != AST_NIL)
    {
      ldata = (Loop_list *) dg_get_info(infoPtr, loop, type_ref);
      slist = dt_slist(dt, ldata, node, cfgInstance);
    }
  else
    {
      slist = dt_slist(dt, NULL, node, cfgInstance);
    }

  dg_put_info(infoPtr, node, type_ref, (Generic)slist);
  return WALK_CONTINUE;		/* no need to look at components */
}


/*-----------------------------------------------------------------------

	dt_clear_loopref()	Clear loop & reference info

*/
static int
dt_clear_loopref(AST_INDEX node, SideInfo *infoPtr)
{
  if (is_loop(node) || is_subscript(node))
    dg_put_info( infoPtr, node, type_ref, NO_REF);

  return WALK_CONTINUE;
}

/*-----------------------------------------------------------------------

	dt_edge_test()		Calculate DT info for DG edge

*/

static void
dt_edge_test(DG_Instance *dg, DT_info *dt, SideInfo *infoPtr, DG_Edge *Earray, 
             EDGE_INDEX Eindex, int level, CfgInstance cfgInstance)
{
  DG_Edge *Edge;

  Edge = &Earray[Eindex];

  /*--------------------------------------*/
  /* handles only simple data dependences */

  if (Edge->type != dg_true   && Edge->type != dg_anti &&
      Edge->type != dg_output && Edge->type != dg_input)
    {
      return;
    }

  /*---------------------------------------*/
  /* apply test, attach info to edge, then */
  /* decide whether edge should exist	   */

  dt_test(dt, infoPtr, cfgInstance, Edge, 
	  dt_ast_loop(Edge->src), dt_ast_loop(Edge->sink), false);

  if( dg_get_local_analysis(dg) && dt_check_edge(Edge))
    {
      dg_delete_free_edge( dg, Eindex);	/* if no edge, delete 	*/
    }
  else if (Edge->dt_type == DT_NONE)
    {
      /* gotta keep edges from graph/index file, so make up something */

      Edge->dt_type = DT_NONLIN | DT_NOPROVE | 10 | 10 << DT_LBITS;
      dt_dep_all( dt, Edge);
      Edge->dt_str = dt_ssave("", dt);
    }
  else if (Edge->dt_type != DT_UNKNOWN)	/* if DT info		*/
    {
      if( dg_get_local_analysis(dg) || dg_get_set_interchange(dg) )
	{
	  /* update loop interchange/sensitive bits	*/
	  dt_set_intchg(Edge, level);
	}
    }
}
 

/*-----------------------------------------------------------------------

	dt_loop()	Convert a loop header to Loop_data

	Note:	Assume no reversed/triangular loops for now (for simplicity)

		Assign conservative values to symbolics. 
		Let rest of module know that symbolics were encountered.

	Returns	NULL	if cannot analyze loop
			DT_NONE if loop not executed
			pointer to created loop data otherwise

*/
static Loop_list*
dt_loop(AST_INDEX node, DT_info *dt, SideInfo *infoPtr, LooprefParams *looprefParams)
{
  AST_INDEX	 temp;
  int		 result;
  Loop_data	*lptr;
  Loop_list	*outer_loops, *this_loop;

  if (loop_level(node) >= MAXLOOP)
    die_with_message("dt_loop(): max loop depth exceeded");

  this_loop = dt_alloc_loop( dt );

  /*------------------------------------------*/
  /* find Loop_list of enclosing loop, if any */

  temp = tree_out(node);
  while ((temp != AST_NIL) && NOT(is_loop(temp)))
    temp = tree_out(temp);

  if (is_loop(temp))
    {
      outer_loops = (Loop_list *) dg_get_info(infoPtr, temp, type_ref);
      if (outer_loops == NO_REF)
	die_with_message("dt_loop(): missing loop list");

      memcpy(this_loop, outer_loops, sizeof(Loop_list));
    }
  else
    {
      bzero((char *)this_loop, sizeof(Loop_list));
    }

  /* point lptr into appropriate element of the Loop_data array */

  lptr = &this_loop->loops[this_loop->level++];

  /*---------------------------------*/
  /* initialize values for Loop_data */

  lptr->loop_index = node;	/* save index of loop */
  lptr->rev = false;		/* default            */

  /*----------------------------------*/
  /* get control expression from loop */

  node = gen_get_loop_control(node);

  if (!is_inductive(node))	/* not inductive loop */
  {
    /* 2/7/94 NN/RvH: Try to prevent dependence analyzer
     *                from blowing up when while-loops are
     *                encountered.
     *       WARNING: while-loops are analyzed VERY conservatively.
     */
    /* die_with_message("dt_loop(): not inductive do loop");
     * return NULL;
     */
    lptr->ivar      = "";
    lptr->step.type = Expr_constant;
    lptr->step.val  = 1;
    lptr->step.ast  = AST_NIL;
    lptr->lo.type   = Expr_complex;
    lptr->lo.ast    = AST_NIL;
    lptr->lo.val    = MININT; 
    lptr->up.type   = Expr_complex;
    lptr->up.ast    = AST_NIL;
    lptr->up.val    = MAXINT; 
  }
  else
  {
    /*-------------------------------------------------*/
    /* dissect inductive DO loop to build up Loop_data */

    /*--------------------------------*/
    /* get name of induction variable */

    temp = gen_INDUCTIVE_get_name(node);
    if (!is_identifier(temp))
    {
      die_with_message("dt_loop(): inductive not identifier");
      return NULL;
    }
    lptr->ivar = gen_get_text(temp);

    /*--------------------------*/
    /* get step (def = 1 or -1) */

    temp = gen_INDUCTIVE_get_rvalue3(node);

    if (temp == AST_NIL)		/* no step specified, default = 1 */
    {
      lptr->step.type = Expr_constant;
      lptr->step.val = 1;
      lptr->step.ast = AST_NIL;
    }
    else 
    {
      /* we don't do symbolic analysis of step, since this is very rare */
      convert_to_Expr(temp, &lptr->step, this_loop);

      if (lptr->step.type == Expr_constant)
      {
	if (!lptr->step.val)
	{
	  printf("dt_loop(): step found with size 0\n");
	  lptr->step.val = 1; /* default step */
	}
      }
      else
      {
	lptr->step.val = 1;	/* default step */
      }
    }
    
    /*------------------------------------*/
    /* get lower bound (default = MININT)	*/

    temp = gen_INDUCTIVE_get_rvalue1(node);
    
    if (looprefParams->cfgInstance)
      /* if cfgInstance is valid, use symbolic analysis */
      cfgval_dep_parse_loop_bound(looprefParams->cfgInstance, 
				  this_loop->level, temp, 
				  &(lptr->lo), &(lptr->lo_val), &(lptr->lo_vec));
    else
      /* otherwise, do the old style imprecise analysis */
      convert_to_Expr(temp, &lptr->lo, this_loop);
    
    if (lptr->lo.type != Expr_constant)
    {
      /* symbolic lower bound, const step */
      if (lptr->step.type == Expr_constant)
	lptr->lo.val = (lptr->step.val > 0) ? MININT : MAXINT;
      
      /* symbolic lower bound and step */
      else
	lptr->lo.val 	= MININT; 
    }

    /*------------------------------------*/
    /* get upper bound (default = MAXINT)	*/

    temp = gen_INDUCTIVE_get_rvalue2(node);
    
    if (looprefParams->cfgInstance)
      /* if cfgInstance is valid, use symbolic analysis */
      cfgval_dep_parse_loop_bound(looprefParams->cfgInstance, 
				  this_loop->level, temp, 
				  &(lptr->up), &(lptr->up_val), &(lptr->up_vec));
    else
      /* otherwise, do the old style imprecise analysis */
      convert_to_Expr(temp, &lptr->up, this_loop);
    
    if (lptr->up.type != Expr_constant)
    {
      /* symbolic upper bound, const step */
      if (lptr->step.type == Expr_constant)
	lptr->up.val = (lptr->step.val > 0) ? MAXINT : MININT;
      
      /* symbolic upper bound and step */
      else 
	lptr->up.val 	= MAXINT; /* mpal:910717	*/
    }
    
    /*-------------------------------------------------*/
    /* if symbolic step & at least one symbolic bound, */
    /* range becomes meaningless, so set to infinity   */
    
    if (lptr->step.type != Expr_constant) 
    {
      if (lptr->lo.val == MININT) 
	lptr->up.val = MAXINT; 
      
      else if (lptr->up.val == MAXINT)
	lptr->lo.val = MININT; 
    }
    
    /* constant step, massage loop bounds with dt_step() */
    
    else if (dt_step(lptr))	/* adjust loop bounds */
    {
      printf("dt_loop(): unexecutable loop found\n");
    }
  }

  return this_loop;
}



/*-----------------------------------------------------------------------

	dt_check_edge()		Check status of DG edge based on DT info

	Returns 		true if edge should be deleted
				false otherwise

*/
static Boolean
dt_check_edge(DG_Edge *Edge)
{
  int i;
  int dir;

  /*------------------------------*/
  /* check that dependence exists */

  if (Edge->dt_type == DT_NONE)	/* delete edge if no dep	*/
    return true;

  if (Edge->dt_type == DT_UNKNOWN) /* Dunno, guess			*/
    return false;		/* I'd better give up	*/

  /*--------------------------------------*/
  /* check loop independent dependence	*/

  if (Edge->level == LOOP_INDEPENDENT)
    {
      /* delete edge if:									*/
      /*  1) loop indep dep not possible					*/

      if (!gen_is_dt_EQ(Edge))
	return true;

      /*  2) we're looking at single statement output dep	*/

      if ((Edge->type == dg_output) && (Edge->src == Edge->sink))
	return true;

      /*  3) we're looking at single statement anti dep	*/

      if ((Edge->type == dg_anti) &&
	  (dt_ast_stmt(Edge->src) == dt_ast_stmt(Edge->sink)))
	return true;
    }

  /*--------------------------------------*/
  /* check loop carried dependences		*/

  /* if entry in vector is =, then no LC deps at that level	*/

  if (!(dir = gen_get_dt_DIS(Edge, Edge->level)))
    return true;	

  for (i = 1; i < Edge->level; i++) /* find level of dep	*/
    {
      dir = gen_get_dt_DIS(Edge, i);

      /* if distance found at this level, or if no loop indep */
      /* dep dir at this level, then no dependence deeper		*/

      if (dir && (!gen_is_dt_DIR(dir) || (dir == DDATA_LT) || 
		  (dir == DDATA_GT) || (dir == DDATA_NE)))
	return true;		/* dep here but no deeper	*/
    }

  return false;			/* since we already checked for = here	*/
}


/*-----------------------------------------------------------------------

	convert_to_Expr()	Converts AST node into equivalent Expr struct

	Expr may be one of the following types:

		Expr_simple_sym
		Expr_constant
		Expr_linear
		Expr_invocation
		Expr_linear_ivar_only
		Expr_index_var
		Expr_complex

	In the future use Paco's symbolic analysis to get precise info
	on constants, auxiliary induction variables & other symbolic info

	if SYMBOLIC ast node, 
	check the following :             
		  1. IDENTIFIER ?                   
		  2. Contains an INDEX VARIABLE?    
		  3. LINEAR EXPRESSION?             
		  4. NON LINEAR EXPRESSION?         
		  5. INVOCATION?                    
		-------------------------------------
		 store the following :              
		 1. The AST NUMBER in all cases     
		 2. If IDENTIFIER, store string     
		 3. Whether it contains INDEX vars  
      
*/
void
convert_to_Expr(AST_INDEX node, Expr *expr, Loop_list *loop_nest)
{
  int i;
  Boolean is_ivar = true;

  expr->ast = node;
  expr->str = NULL;

  /* is it a constant ? */

  if (NOT(ast_eval(node, &expr->val)))
  {
    expr->type = Expr_constant;
  }

  /* is it an index variable or a simple symbolic? */

  else if (is_identifier(node))
  {
    expr->str = gen_get_text(node);
    for (i = 0; i < loop_nest->level; ++i)
    {
      if (!strcmp(loop_nest->loops[i].ivar, expr->str))
      {
        expr->type = Expr_index_var;
        return;
      }
    }
    expr->type = Expr_simple_sym;
  }

  /* is it an invocation? */

  else if (is_invocation(node))
  {
    expr->type = Expr_invocation;
  }

  /* is it a linear expression with the non constant term as an index var */

  else if (is_linear(loop_nest, node, &is_ivar))
  {
      expr->type = is_ivar ? Expr_linear_ivar_only : Expr_linear;
  }

  /* too complex for previous simple classifications */

  else
  {
    expr->type = Expr_complex;
  }
}


/*--------------------------------------------------------
  static Boolean is_linear()
   return true if it is a linear expression
 else
   return false
---------------------------------------------------------*/

static Boolean
is_linear(Loop_list *loop_nest, AST_INDEX node, Boolean *is_ivar_only)
  /* AST_INDEX node;  subscript expr         */
{
  AST_INDEX term1;
  AST_INDEX term2;
  int coeff;
  char *ivar;
  Boolean done;
  int i;

  if (!is_constant(node))
  {
    if (is_identifier(node))
    {
      ivar = gen_get_text(node);
      done = false;
      for (i = 0; i < loop_nest->level; ++i)
      {
        if (!strcmp(loop_nest->loops[i].ivar, ivar))
          done = true;
      }
      if (!done)
        *is_ivar_only = false;
     return(true);
		}
    /*---------------------------------------------------*/
    /* plus or minus */

    else if (is_binary_plus(node))
    {
      term1 = gen_BINARY_PLUS_get_rvalue1(node);
      if (!is_linear(loop_nest, term1, is_ivar_only))
        return (false);

      term2 = gen_BINARY_PLUS_get_rvalue2(node);
      is_linear(loop_nest, term2, is_ivar_only);
    }

    /*---------------------------------------------------*/

    else if (is_binary_minus(node))
    {
      term1 = gen_BINARY_MINUS_get_rvalue1(node);
      if (!is_linear(loop_nest, term1, is_ivar_only))
        return (false);

      term2 = gen_BINARY_MINUS_get_rvalue2(node);
      is_linear(loop_nest, term2, is_ivar_only);

    }

    /*---------------------------------------------------*/
    /* multiply         */

    else if (is_binary_times(node))
    {
      term1 = gen_BINARY_TIMES_get_rvalue1(node);
      term2 = gen_BINARY_TIMES_get_rvalue2(node);

      if (NOT(ast_eval(term1, &coeff)))        /* 1st term has const val */

        if (!is_identifier(term2))  /* 2nd term is id */
          return false;             /* no idea what 2nd term is, give up   */
        else
          is_linear(loop_nest, term2, is_ivar_only);

      else if (NOT(ast_eval(term2, &coeff)))  /* 2nd term has const val */
        if (!is_identifier(term1))
          return false;        /* no idea what 1st term is, give up    */
        else
          is_linear(loop_nest, term1, is_ivar_only);
      else
        return false;        /* neither term const, give up         */
		}

    /*---------------------------------------------------*/
    /* not in proper normalized form         */

    else
      return false;
	}
  return true;           /* found linear subscript         */
}


/*-----------------------------------------------------------------------

	dt_common_loops()

	Find level of common loops (loops enclosing both refs)

*/

static int
dt_common_loops(Loop_list *l1, Loop_list *l2)
	/* Loop_list *l1;	    src	*/
	/* Loop_list *l2;	    sink */
{
	int lvl;

	lvl = 0;

	while (true)
	{
		/* eventually will run out of loops for one ref		*/
		/* or find loop not shared by both refs			*/

		if ((l1->level <= lvl) || (l2->level <= lvl) ||
		    (l1->loops[lvl].loop_index != l2->loops[lvl].loop_index))
			return lvl;

		lvl++;
	}
}


/*----------------------------------------------------------------

	dt_step()	massage loop bounds for step != 1

	Returns 	true	if loop not executed
			false	otherwise

*/
static Boolean
dt_step(Loop_data *loops)
{
  int up_b;
  int lo_b;
  int step;
  int temp;
  int *ptr;

  up_b = loops->up.val;
  lo_b = loops->lo.val;
  step = loops->step.val;

  /*-------------------------------------------------------------*/
  /* if symbolic step found, make sure that step/loop is correct */

  if (loops->step.type != Expr_constant)
    {
      step = 1;

      if (up_b < lo_b)		/* reverse loop		*/
	{
	  temp = up_b;
	  up_b = lo_b;
	  lo_b = temp;

	  loops->rev = true;	/* remember reverse	*/
	}
    }

  /*----------------------------------------------------------------*/
  /* May need to adjust bounds if |step| > 1 (for better precision) */

  else if (step >= 1)
    {
      if (up_b < lo_b)
	return true;		/* loop not executed	*/
	
      /* adjust bounds if step > 1 for more accuracy	*/
	
      if ((step > 1) && (up_b != MAXINT) && (lo_b != MININT))
	{
	  up_b -= (up_b - lo_b) % step;
	}
    }
	
  /*------------------------------------------------------*/
  /* Partially normalize loop if needed (to get step > 0) */

  else if (step < 0)
    {
      if (up_b > lo_b)
	return true;		/* loop not executed	*/
	
      step = -step;		/* reverse loop		*/
      temp = up_b;
      up_b = lo_b;
      lo_b = temp;
      
      temp = loops->up_val;
      loops->up_val = loops->lo_val;
      loops->lo_val = temp;

      ptr = loops->up_vec;
      loops->up_vec = loops->lo_vec;
      loops->lo_vec = ptr;

	
      loops->rev = true;	/* remember reverse	*/
	
      /* adjust bounds if step > 1 for more accuracy	*/
      /* note: semantics are different for rev loop	*/
	
      if ((step > 1) && (up_b != MAXINT) && (lo_b != MININT))
	lo_b += (up_b - lo_b) % step;
    }

  /*------------------*/
  /* store new values	*/

  loops->up.val = up_b;
  loops->lo.val = lo_b;
  loops->step.val = step;

  return false;			/* no errors	*/
}


/*-----------------------------------------------------------------------

	dt_slist()		Analyze and convert list of subscripts

	Returns:	true	if nonlinear subscripts found
				false	otherwise

*/
static Subs_list *
dt_slist(DT_info *dt, Loop_list *ldata, AST_INDEX subs, CfgInstance cfgInstance)
{
  Subs_list *sdata;		/* subscript data	*/
  int i;
  int dims;

  sdata = dt_alloc_ref( dt );	   /* allocate ref      */
  bzero((char *)sdata, sizeof(Subs_list)); /* init subs data	*/
  dims = 0;

  /*--------------------------------*/
  /* look at each subscript in list */

  subs = dt_ast_sub(subs);	/* get to subs node	*/

  subs = list_first(gen_SUBSCRIPT_get_rvalue_LIST(subs));

  while (subs != AST_NIL)
    {
      sdata->subs[dims].stype = SUBS_ZIV;
      sdata->subs[dims].sym = subs;
      sdata->subs[dims].symbolic_constant = VAL_ZERO;

      if (dt_sub(subs, sdata->subs + dims, ldata, true))
	{
	  /* subscript was symbolic or too complex for dt_sub */

	  if (cfgInstance && ldata)
	    /* if cfgInstance is valid, try symbolic analysis */
	    cfgval_dep_parse_sub(cfgInstance, sdata->subs + dims,ldata->level);
	  else
	    /* otherwise, just give up */
	    sdata->subs[dims].stype = SUBS_SYM;
	}

      dims++;
      subs = list_next(subs);	/* next on list	*/
    }

  sdata->dims = dims;		/* total # of dims in array ref */
  sdata->loop_nest = ldata;	/* enclosing loop nest */

  return sdata;
}

/*-----------------------------------------------------------------------

	dt_ivar()		Mark induction var instance in subscript structure

	Returns:	true if not induction variable
*/

static Boolean
dt_ivar(char *ivar, int coeff, Loop_list *ldata, Subs_data *sdata, Boolean plus)
    /* char *ivar;	    index of induction ar	*/
    /* int coeff;	    its coefficient	        */
    /* Loop_list *ldata;    loop data		       	*/
    /* Subs_data *sdata;    data on subscript		*/
    /* Boolean plus;	    sign of data       		*/
{
	int i;

	if (!ldata)
		return true;

	/* loop through and compare ivar with all induction variables	*/

	for (i = 0; i < ldata->level; i++)
	{
		if (STREQ(ivar, ldata->loops[i].ivar))	/* found	*/
		{
			sdata->coeffs[i] += plus ? coeff : -coeff;

			if (sdata->stype == SUBS_ZIV)
				sdata->stype = i;
			else if ((sdata->stype <= SUBS_SIV) && (sdata->stype != i))
				sdata->stype = SUBS_MIV;
			return false;
		}
	}

	return true;					/* not found	*/
}


/*-----------------------------------------------------------------------

	dt_dep_all()	Create Dt_info->data & str w/ dep on all levels

*/

static void 
dt_dep_all(DT_info *dt, DG_Edge *Edge)
{
	static char *alldir[] =
	{
	"()",
	"(*)",
	"(*,*)",
	"(*,*,*)",
	"(*,*,*,*)",
	"(*,*,*,*,*)",
	"(*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*,*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*)",
	"(*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*)",
	};
	int level;
	int i;

	level = gen_get_dt_CLVL(Edge);

	/*----------*/
	/* set str	*/

	Edge->dt_str = dt_ssave(alldir[level], dt );

	/*----------*/
	/* set data	*/

	for (i = 0; i < level; i++)
		Edge->dt_data[i] = DDATA_ANY;
}



