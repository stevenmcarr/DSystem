/* $Id: dt_build.C,v 1.3 2000/05/18 21:28:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*-----------------------------------------------------------------------

	dt_build.c		quick and dirty walk routines to build dg

*/

#include <memory.h>
#include <string.h>

#include <libs/support/misc/general.h>
#include <libs/ipAnalysis/ipInfo/iptypes.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>

/* this include file ends up bringing in Arena.h, which contains C++ stuff */
/* #include <libs/ped_cp/memory/include/sr.h> */

#include <libs/moduleAnalysis/dependence/dependenceTest/dt_build.i>

/* necessary to use interprocedural information */
#include <libs/ipAnalysis/problems/modRef/ScalarModRefQuery.h>
#include <libs/ipAnalysis/problems/rsd/AFormalQuery.h>

/* to use cfgInfo, value numbering and ssa */
#include <libs/moduleAnalysis/cfg/cfg.h>

#include <libs/frontEnd/ast/forttypes.h>

/*---------------*/
/* forward decls */

STATIC(int, dg_stmt_refs,(AST_INDEX stmt, int lvl, 
                          Dg_ref_params *r));
STATIC(int, dg_loop_refs,(AST_INDEX stmt, int lvl, 
                          Dg_ref_params *r));
STATIC(int, dg_expr_uses,(AST_INDEX expt, Dg_ref_params *r));
STATIC(void, dg_build_pair,(Dg_ref_params *r, Dg_ref_info *ref1s,
                            Dg_ref_info *ref2s, DepType type));
STATIC(void, dg_build_DG,(Dg_ref_params *r));
STATIC(void, dg_build_LI,(Dg_ref_params *t, FortTree ft));
STATIC(void, dg_build_edges,(Dg_ref_params *r, Dg_loop_info *loop));
STATIC(void, dg_refs_free,(Dg_ref_params *r));
STATIC(void, dg_show_refs,());
STATIC(void, dt_refs_outside,(Dg_ref_info *vref, Dg_loop_info *Loop,
                              Dg_ref_params *r, Slist *sptr));
STATIC(Boolean, dt_reached_use,(Dg_ref_info *vref, 
                                Dg_loop_info *Loop, 
                                Dg_ref_params *r, FortTree ft));
STATIC(void, dt_rev_info,());
STATIC(DG_Edge, *dg_ins_edge,(DG_Instance *dg, DT_info *dt, 
                              SideInfo *infoPtr, AST_INDEX src,
                              AST_INDEX sink, DepType type, 
                              int level, DG_Edge *Edge, AST_INDEX 
                              scr_stmt, AST_INDEX sink_stmt));
STATIC(void, dg_refs,(AST_INDEX tree, Dg_ref_params *r));
STATIC(int, dt_walk_clear_lv,(AST_INDEX expt, Clear_lv_parm *parm));
STATIC(int, dt_walk_clear_edges,(AST_INDEX stmt, int level, 
                                  Clear_edge_parm *params));
STATIC(void, dg_clear,(DG_Instance *dg, DT_info *dt, 
                       SideInfo *infoPtr, AST_INDEX root));
STATIC(void, dg_clear_levelv,());
STATIC(void, dt_rev_info,(DT_info *dt, DG_Edge *Edge));
STATIC(Boolean, dt_refs_overlap,(int offset1, int size1, 
                                 int offset2, int size2));

STATIC(void, dg_invoc_refs,(AST_INDEX invoc, Dg_ref_params *r));
STATIC(void, dg_ip_query_add_ref,(SymDescriptor d, Dg_ref_params *r,
                                  AST_INDEX expr, char *sym,
                                  int callsite_id, 
                                  Boolean callee_intrinsic,
                                  unsigned int param_pos));
STATIC(int, dg_invoc_modref,(SymDescriptor d, int callsite_id,
                             Boolean callee_intrinsic, 
                             AST_INDEX expr, Dg_ref_params *r,
                             unsigned int param_pos));

/********************/
/* global functions */
/********************/

/*----------------------------------------------------------------------------

	dg_build()	Builds DG and LI for entire program from scratch
			Requires that the DG and LI structures already exist.
*/
void
dg_build(AST_INDEX root, FortTree ft, DG_Instance *dg, 
         SideInfo *infoPtr, DT_info *dt, LI_Instance *li, 
         Generic program_callgraph, CfgInfo cfgModule)
{
  Dg_ref_params r;

  r.dg                = dg;
  r.dt                = dt;
  r.li                = li;
  r.infoPtr           = infoPtr;
  r.program_callgraph = program_callgraph;
  r.ft                = ft;
   
  dg_refs(root, &r);
  dg_build_DG( &r );

  /* build control dependences */
  if (cfgModule != NULL)
    {
      dg_build_module_cds(dg, infoPtr, cfgModule);
    }

  dg_build_LI(&r, ft);		/* uses data in r->loop_num, r->loops */
  dg_refs_free(&r);
  dg_set_external_analysis(dg, graph_local); /* did local analysis	*/
} 


/*--------------------------------------------------------------------

	dg_update()		Rebuild DG and LI for selected subtree

*/
void
dg_update(AST_INDEX root, FortTree ft, DG_Instance *dg, 
          SideInfo *infoPtr, DT_info *dt, LI_Instance *li)
{
  Dg_ref_params	 r;

  if( dg_get_external_analysis(dg) == graph_pfc )
    dg_set_external_analysis( dg, graph_updated_pfc); 

  r.dg	= dg;
  r.dt	= dt;
  r.li	= li;
  r.infoPtr= infoPtr;
  r.ft	= ft;			/*  added by cp@km21.zfe.siemens.de, 9305*/
  r.current_proc=gen_get_text(get_name_in_entry(find_scope(root))); /*cp*/
  r.sym_descriptor = ft_SymGetTable(ft, r.current_proc);

  dg_clear(r.dg, r.dt, r.infoPtr, root); /* clear existing edges	*/
  dg_refs(root, &r);			/* build new dependence graph	*/
  dg_build_DG(&r);
  dg_build_LI(&r, ft);
  dg_refs_free(&r);
}


/*-----------------------------------------------------------------------

	dt_copy_info()	Makes copy of DT info from one DG edge to another

*/
void
dt_copy_info(DT_info *dt, DG_Edge *from, DG_Edge *to)
{
  to->dt_type = from->dt_type;
  memcpy(to->dt_data, from->dt_data, sizeof(int) * MAXLOOP);
  to->dt_str = dt_ssave(from->dt_str, dt);
}

/*-----------------------------------------------------------------------

	dg_ast_dim()	Finds out # of dims for a variable

	Return:		# of dims found (0 = scalar)

*/
int
dg_ast_dim(AST_INDEX node)
{
  int subs;	/* # of subs found */

  /*----------------------------*/
  /* first check whether scalar	*/

  node = tree_out(node);
  if (!is_subscript(node))
    return 0;

  /*----------------------*/
  /* find # of subscripts */

  subs = 0;
  node = list_first(gen_SUBSCRIPT_get_rvalue_LIST(node));

  while (node != AST_NIL)
    {
      subs++;
      node = list_next(node);	/* next on list	*/
    }

  return subs;
}


/*-----------------------------------------------------------------------

	dt_info_str()		Makes string of dependence info

	Assume that type & data fields are set, creates string.
*/
void
dt_info_str(DT_info *dt, DG_Edge *Edge)
{
  char		cbuf[128];	/* use 128 for now	*/
  char		tbuf[32];	/* use 32 for now	*/
  int         	i;
  int		max_level;

  /*--------------------------------------*/
  /* build string for dependence vector	*/

  max_level = gen_get_dt_CLVL(Edge);
  cbuf[0] = '(';
  cbuf[1] = 0;
	
  for (i = 0; i < max_level; i++)
    {
      switch (Edge->dt_data[i])
	{
	case DDATA_LT:
	  strcat(cbuf,"<");
	  break;

	case DDATA_GT:
	  strcat(cbuf,">");
	  break;

	case DDATA_LE:
	  strcat(cbuf,"<=");
	  break;

	case DDATA_GE:
	  strcat(cbuf,">=");
	  break;

	case DDATA_NE:
	  strcat(cbuf,"<>");
	  break;

	case DDATA_ANY:
	  strcat(cbuf,"*");
	  break;

	case 0:
	  strcat(cbuf,"=");
	  break;

	case DDATA_ERROR:
	  strcat(cbuf,"?");
	  break;

	default:
	  sprintf(tbuf,"%d", Edge->dt_data[i]);
	  strcat(cbuf, tbuf);
	  break;
	}

      if (i < max_level - 1)
	strcat(cbuf,",");
    }

  strcat(cbuf,")");

  /*--------------------------------------*/
  /* mark dependences proven exactly		*/

  if (!gen_is_dt_NOPROVE(Edge))
    strcat(cbuf," !");

  /*--------------------------------------*/
  /* alloc mem for new str & store 		*/

  Edge->dt_str = dt_ssave(cbuf, dt );
}


/*-----------------------------------------------------------------------

	dg_ref_info_and_count()	Given the AST_INDEX of a loop,

	Returns:	The Dg_ref_info structures for each reference,
			and the number of references within that loop.

*/
void
dg_ref_info_and_count(FortTree ft, AST_INDEX loop, int *count, 
                      Dg_ref_info **refArray)
{
  Dg_ref_params r;

  r.ft = ft;
  r.current_proc=gen_get_text(get_name_in_entry(find_scope(loop))); /*cp*/
  r.sym_descriptor = ft_SymGetTable(ft, r.current_proc);

  dg_refs(loop, &r);		/* get refs in loop */
  *count = r.ref_num;
  *refArray = (Dg_ref_info *) get_mem(sizeof(Dg_ref_info) * r.ref_num, 
					  "dg_ref_info_and_count");

  memcpy((void *)*refArray, (void *)r.refs, sizeof(Dg_ref_info) * r.ref_num);

  dg_refs_free( &r );		/* Free the Dg_ref_params memory	*/
}

/*-----------------------------------------------------------------------

	dg_ref_info_free()	Free the memory allocated to hold
				an array of Dg_ref_info structures.

*/
void
dg_ref_info_free(Dg_ref_info *refArray)
{
  free_mem(refArray);
}



#ifdef UNUSED_CODE

/*-----------------------------------------------------------------------

	fst_AstDims()  	Use symbol table to find # of dims in variable
			modified from dg_conservative_rsd_string() in dt_build2.C

*/
int
fst_AstDims(FortTree ft, AST_INDEX node, char *name)
{
  SymDescriptor d;
  int nindex;

  d = ft_SymGetTable(ft, gen_get_text(get_name_in_entry(find_scope(node))));

  nindex = fst_QueryIndex(d, name);

  if (!fst_index_is_valid(nindex))
    return 0;

  return fst_GetFieldByIndex(d, nindex, SYMTAB_NUM_DIMS); 
}

#endif


/*******************/
/* local functions */
/*******************/

/*-----------------------------------------------------------------------

	dg_refs()	Find all variables referenced in AST
			The list includes any induction variables used 
			Separated into def & uses lists

	Returns:	Pointer to Dg_ref_params structure holding results

*/

static void
dg_refs(AST_INDEX tree, Dg_ref_params *r)
     /* AST_INDEX tree;		portion of AST to be checked	*/
{

  /*-----------------------*/
  /* initialize parameters */

  r->ref_num	= 0;
  r->ref_size	= R_REFS;
  r->stmt_num	= 0;
  r->stmt_size	= R_STMTS;
  r->loop_num	= 0;
  r->loop_size	= R_LOOPS;
  r->loop_cur_level = 0;
  r->func_num	= 0;
  r->func_size	= R_FUNCS;

  r->guards = 0;		/* no structured guards initially	*/

  r->refs  = (Dg_ref_info *) 
    get_mem(sizeof(Dg_ref_info) * R_REFS, "dg_refs");
  r->stmts = (Dg_stmt_info *) 
    get_mem(sizeof(Dg_stmt_info) * R_STMTS, "dg_refs");
  r->loops = (Dg_loop_info *) 
    get_mem(sizeof(Dg_loop_info) * R_LOOPS, "dg_refs");
  r->funcs = (Dg_func_info *) 
    get_mem(sizeof(Dg_func_info) * R_FUNCS, "dg_refs");


  if (!r->refs || !r->loops || !r->stmts || !r->funcs) 
    die_with_message("dg_refs(): out of memory\n");

  /*------------*/
  /* go on walk	*/

  walk_statements(tree, LEVEL1, (WK_STMT_CLBACK)dg_stmt_refs, (WK_STMT_CLBACK)dg_loop_refs, (Generic) r);
}


/*-----------------------------------------------------------------------

	dg_refs_free()		Free result from dg_refs()

*/
static void
dg_refs_free(Dg_ref_params *r)
{
  int i, j;
  Dg_loop_info	   *Loop;
  Dg_loop_var_info *eqClass;

  /*
   * for (i = 0; i < r->ref_num; i++) 
   *   sfree(r->refs[i].sym);
   */

  for (i = 0; i < r->loop_num; i++)
    {
      Loop = r->loops + i;
      for (j = 0; j < Loop->loop_var_num; j++)
	{
	  eqClass = Loop->loop_vars + j;
	  free_mem((void*)eqClass->var_refs);
	  free_mem((void*)eqClass->var_names);
	}
      free_mem((void*)Loop->loop_vars);
    }
  
/****************************************************************** 
  This needs to be changed so that vectors allocated for names 
  within a loop are also freed!
*******************************************************************/

  free_mem((void*)r->refs);
  free_mem((void*)r->funcs);
  free_mem((void*)r->loops);
  free_mem((void*)r->stmts);
}


/*-----------------------------------------------------------------------

	dg_stmt_refs()		Find all variables referenced in AST

	Returns:		Code for walk_statements()

	Pre action performed by walk_statements()
*/
static int
dg_stmt_refs(AST_INDEX stmt, int lvl, Dg_ref_params *r)
     /* AST_INDEX stmt;		statement being examined	*/
     /* int lvl;		loop nesting level		*/
     /* Dg_ref_params *r;	contains ref info		*/
{
  AST_INDEX	 expr1;
  AST_INDEX	 expr2;
  Dg_loop_info	*Loop;
  Dg_func_info	*Func;
  

  if (is_subroutine(stmt) || is_function(stmt) || is_program(stmt)) 
    {
      /* save routine name so caller of an invocation can be determined */
      r->current_proc = gen_get_text(get_name_in_entry(stmt));
      r->sym_descriptor = ft_SymGetTable(r->ft, r->current_proc);

      /*----------------------------------*/
      /* Create storage for this function */
    
      if (r->func_num == r->func_size) 
	{
	  r->func_size <<= 1;
	  r->funcs = 
	    (Dg_func_info *)reget_mem((void*)r->funcs, 
				      sizeof(Dg_func_info) * r->func_size, 
				      "dg_stmt_refs");
	  if (!r->funcs)
	    die_with_message("dg_stmt_refs(): out of memory\n");
	}

      /*-------------------------------*/
      /* save info about this function */
    
      Func = r->funcs + r->func_num;
    
      Func->func_hdr   = stmt;	
      Func->func_name  = r->current_proc;
      Func->func_start = r->ref_num;	
      Func->func_end   = UNUSED;	
      Func->first_loop = r->loop_num;
      Func->last_loop  = UNUSED;
      Func->cfg_inst = (CfgInstance)dg_get_info(r->infoPtr, stmt,type_cfg_inst);

      (r->func_num)++;
      return WALK_CONTINUE;
    } 

  else if (is_executable_stmt(stmt)) 
    {
      /*---------------------------------*/
      /* store all executable statements */
    
      if (r->stmt_num == r->stmt_size) 
	{
	  r->stmt_size <<= 1;
	  r->stmts = 
	    (Dg_stmt_info *) reget_mem((void*)r->stmts, 
				       sizeof(Dg_stmt_info) * r->stmt_size, 
				       "dg_stmt_refs");
	  if (!r->stmts)
	    die_with_message("dg_stmt_r(): out of memory\n");
	}
    
      r->stmts[r->stmt_num].node = stmt;	
      r->stmts[r->stmt_num++].level = lvl;	
    
      if (struct_guard(stmt))		/* remember any guards seen */
	r->guards++;
    }
  
  /*-------------------------------------------------------*/
  /* special treatment of do, since we want induction vars */
  
  if (is_loop(stmt)) 
    {
      /*--------------------------------------------*/
      /* first create storage for DO statement      */
    
      if (r->loop_num == r->loop_size) 
	{
	  r->loop_size <<= 1;
	  r->loops = 
	    (Dg_loop_info *) reget_mem((void*)r->loops, 
				       sizeof(Dg_loop_info) * r->loop_size, 
				       "dg_stmt_refs");
	  if (!r->loops)
	    die_with_message("dg_stmt_refs(): out of memory\n");
	}
    
      /*-------------------------------------*/
      /* now examine do statement components */
    
      expr1 = gen_get_loop_control(stmt);
    
      /* 2/7/94 NN/RvH: Try to prevent dependence analyzer
       *                from blowing up when while-loops are
       *                encountered.
       *       WARNING: while-loops are analyzed VERY conservatively.
       */

      if (is_inductive(expr1))
      {
	/*----------------------------------------------*/
	/* don't add def for induction variable	      */
	
	/*----------------------------------------------*/
	/* walk over lower, upper, and step expressions	*/
	
	if ((expr2 = gen_INDUCTIVE_get_rvalue1(expr1)) != AST_NIL)
	  walk_expression(expr2, (WK_EXPR_CLBACK)dg_expr_uses, NULL, (Generic) r);
	
	if ((expr2 = gen_INDUCTIVE_get_rvalue2(expr1)) != AST_NIL)
	  walk_expression(expr2, (WK_EXPR_CLBACK)dg_expr_uses, NULL, (Generic) r);
	
	if ((expr2 = gen_INDUCTIVE_get_rvalue3(expr1)) != AST_NIL)
	  walk_expression(expr2, (WK_EXPR_CLBACK)dg_expr_uses, NULL, (Generic) r);
	
	/*----------------------------------------------*/
	/* store away inductive variable & other data   */
	
	expr2 = gen_INDUCTIVE_get_name(expr1);
	Loop = r->loops + r->loop_num;
	Loop->ivar = expr2;
	Loop->ivar_sym = gen_get_text(expr2);
	Loop->ivar_index = fst_QueryIndex(r->sym_descriptor,gen_get_text(expr2));
      }
      else
      {
	if (expr1 != AST_NIL)
	  walk_expression(expr1, (WK_EXPR_CLBACK)dg_expr_uses, NULL, (Generic) r);
	Loop = r->loops + r->loop_num;
	Loop->ivar = AST_NIL;
	Loop->ivar_sym = "";
	Loop->ivar_index = SYM_INVALID_INDEX;
      }
	
      Loop->loop_hdr = stmt;	
      Loop->loop_level = lvl;	
      Loop->loop_start = r->ref_num;	
      Loop->loop_end = UNUSED;	

      Loop->loop_var_num  = 0;
      Loop->loop_var_size = R_LOOP_VARS;
      Loop->loop_vars  = (Dg_loop_var_info *) 
	get_mem(sizeof(Dg_loop_var_info) * R_LOOP_VARS, "dg_stmt_refs");
      if (!Loop->loop_vars) 
	die_with_message("dg_stmt_refs(): out of memory\n");
    
      /* save index of cur loop	*/
      r->loop_cur[r->loop_cur_level++] = r->loop_num++;
    }
  
  /*---------------------------------------------------------*/
  /* special treatment of assign, since we separate def/uses */
  
  else if (is_assignment(stmt)) 
    {
      /*--------------------------------------------------*/
      /* mark variables used first, then variable defined */
    
      expr1 = gen_ASSIGNMENT_get_lvalue(stmt);	/* get lhv	*/
      expr2 = gen_ASSIGNMENT_get_rvalue(stmt);	/* check rhv	*/
      walk_expression(expr2, (WK_EXPR_CLBACK)dg_expr_uses, NULL, (Generic) r);
    
      if (is_subscript(expr1)) 
	{
	  /*------------------------------------------------------*/
	  /* array variables may include uses in subscripts	  */
	  /* so if array def, get uses in subscript expression(s) */
      
	  expr2 = gen_SUBSCRIPT_get_rvalue_LIST(expr1);	/* check rhv	*/
	  walk_expression(expr2, (WK_EXPR_CLBACK)dg_expr_uses, NULL, (Generic) r);
      
	  expr1 = gen_SUBSCRIPT_get_name(expr1);	/* get lhv	*/
	}
    
      /* add def last */
    
      dg_add_ref(r, expr1, gen_get_text(expr1), GUARDED | T_DEF);
    }
  
  /*-----------------------------------------------*/
  /* io statement, get & walk over component exprs */
  
  else if (io_stmt(stmt)) 
    {
      walk_expression(stmt, (WK_EXPR_CLBACK)dg_expr_uses, NULL, (Generic) r);
      return WALK_CONTINUE;
    }
  
  /*-----------------*/
  /* subroutine call */
  
  else if (is_call(stmt)) 
    {
      dg_invoc_refs(gen_CALL_get_invocation(stmt), r);
      return WALK_CONTINUE;
    }
    
  /*------------------------------------------------------*/
  /* skip most compound stmts, walk over components later */
  
  else if (is_compound(stmt)) 
    {
      if (is_guard(stmt)) 
	walk_expression(gen_GUARD_get_rvalue(stmt), 
		      (WK_EXPR_CLBACK)dg_expr_uses, NULL, (Generic) r);

      else if (is_logical_if(stmt)) 
	walk_expression(gen_LOGICAL_IF_get_rvalue(stmt), 
			(WK_EXPR_CLBACK)dg_expr_uses, NULL, (Generic) r);
    }
  
  /*-----------------------------------------------------------*/
  /* generic single statement, get & walk over component exprs */
  
  else 
    {
      if (get_expressions(stmt, &expr1, &expr2) == UNKNOWN_STATEMENT)
	return WALK_CONTINUE; 
    
      /* procedure names are not references */
    
      if ((expr1 != AST_NIL) && !is_invocation(stmt))
	walk_expression(expr1, (WK_EXPR_CLBACK)dg_expr_uses, NULL, (Generic) r);
    
      if (expr2 != AST_NIL)
	walk_expression(expr2, (WK_EXPR_CLBACK)dg_expr_uses, NULL, (Generic) r);
    }

  return WALK_CONTINUE;
}

/*-----------------------------------------------------------------------

	dg_loop_refs()		Find all variables referenced in loop
						Builds info for LI

	Returns:		Code for walk_statements()

	This function called as post action performed by walk_statements()
*/

static int
dg_loop_refs(AST_INDEX stmt, int lvl, Dg_ref_params *r)
     /* AST_INDEX stmt;		statement being examined	*/
     /* int lvl;		loop nesting level		*/
     /* Dg_ref_params *r;	contains ref info		*/

{
  Dg_func_info	*Func;
  Dg_loop_info	*Loop;

  /*--------------------------------------------*/
  /* if finished function body, mark location	*/

  if( is_subroutine(stmt) || is_function(stmt) || is_program(stmt) ) 
    {
      /* record the last of the variables in this function */
      Func = r->funcs + (r->func_num -1);
      if((Func->func_end != UNUSED) || (Func->last_loop != UNUSED)) 
	{
	  printf("dg_loop_refs(): mismatch function in AST!\n");
	}
      /* store location after last ref and last loop in function */
      Func->func_end  = r->ref_num;
      Func->last_loop = r->loop_num;
    }

  /*--------------------------------------*/
  /* if finished loop body, mark location */
  else if (is_loop(stmt))
    {	  
      /* close last open loop body */
      Loop = r->loops + r->loop_cur[--(r->loop_cur_level)];

      if (Loop->loop_end != UNUSED)
	{
	  printf("dg_loop_refs(): mismatch loop!\n");
	}

      /* store location after last ref in loop	*/
      Loop->loop_end = r->ref_num;
    }

  /*---------------------------------------------*/
  /* if structured guard, decrement count by one */

  if (struct_guard(stmt))
    r->guards--;

  return WALK_CONTINUE;
}

/*-----------------------------------------------------------------------

	dg_invoc_refs()		Find all variables referenced in invocation

	Works as follows:

	if (intrinsic function)
		assume args are referenced, but not defined
	else if (interprocedural info)
		check info to calculate refs & defs
	else
		insert CALL dependence

*/
static void
dg_invoc_refs(AST_INDEX invoc, Dg_ref_params *r)
	/* AST_INDEX invoc;    invocation being examined	*/
	/* Dg_ref_params *r;   contains ref info		*/
{
  Dg_stmt_info *stmt = &(r->stmts[r->stmt_num - 1]);

  /*----------------------------------------------------*/
  /* check whether invocation is of instrinsic function */

  int callsite_id = ft_NodeToNumber(r->ft, invoc);
  AST_INDEX plist = gen_INVOCATION_get_actual_arg_LIST(invoc);
  /*** SymDescriptor d = ft_SymGetTable(r->ft, r->current_proc); ***/
  SymDescriptor d = r->sym_descriptor;
  char *iname = gen_get_text(gen_INVOCATION_get_name(invoc));
  Boolean callee_intrinsic = false;

  assert(d != NULL); /* I must have a symbol table for this routine */ 
  if (builtins_isBuiltinFunction(iname)) {
    fst_index_t callee_index = fst_QueryIndex(d, iname);
    int sc;

    /* there must be an entry for callee */
    assert(fst_index_is_valid(callee_index)); 

    sc = fst_GetFieldByIndex(d, callee_index,SYMTAB_STORAGE_CLASS);
    if (sc & (SC_INTRINSIC | SC_GENERIC)) callee_intrinsic = true;
  }

  if (stmt->level > 1) 
  {
    /*-------------------------------------------------------------*/
    /* if not instrinsic & no IP info, build CALL dependence edges */

    if (!callee_intrinsic && !r->program_callgraph)
    {
      int lvl;
      for (lvl = stmt->level - 1; lvl > 0; lvl--)
        dg_ins_edge(r->dg, r->dt, r->infoPtr, invoc, invoc, dg_call, lvl, NULL, 
		    stmt->node, stmt->node);
    }

    /*-------------------------------------------------------------*/
    /* if not instrinsic & no IP info, build CALL dependence edges */

    else
    { 
      int param_pos = 1;  /* first parameter position: should be a 
			   * defined constant -- JMC 7/92  <HACK> */ 
      for (plist = list_first(plist); plist != AST_NIL; 
	      plist = list_next(plist), param_pos++ ) {
        dg_invoc_modref(d, callsite_id, callee_intrinsic, plist, r, param_pos);
      }

      if (!callee_intrinsic)
        dg_invoc_globals(d, callsite_id, r, invoc);
    }
  }
}

/*-----------------------------------------------------------------------

	dg_invoc_modref()	Mark actuals in an invocation using 
	                        interprocedural information
*/

static int
dg_invoc_modref(SymDescriptor d, int callsite_id, 
                Boolean callee_intrinsic, AST_INDEX expr, 
                Dg_ref_params *r, unsigned int param_pos)
  /* SymDescriptor d;                symbol table for caller */
  /* int callsite_id;                callsite id of invocation */
  /* Boolean callee_intrinsic;       is callee an intrinsic    */
  /* AST_INDEX expr;		     expression being examined	*/
  /* Dg_ref_params *r;		     contains ref info	*/
  /* unsigned int param_pos;         parameter position of actual */
{
  int i;
  char *sym;
  char *sym2;
  
  if (is_identifier(expr)) {
    /*--------------------------------------------*/
    /* filter out active loop induction variables */
    
     
    sym = gen_get_text(expr); 
    
    for (i = 0; i < r->loop_cur_level; i++) {
    /* get symbol for induction var at this loop level */
      
       sym2 = r->loops[r->loop_cur[i]].ivar_sym; 
       if STREQ(sym, sym2) return 0;
      
    }
    dg_ip_query_add_ref(d, r, expr, sym, callsite_id, callee_intrinsic, 
			param_pos);  /* add info	*/
  }
  else if (is_subscript(expr)) {
    AST_INDEX name = gen_SUBSCRIPT_get_name(expr);
    char *sym = gen_get_text(name);
    walk_expression(gen_SUBSCRIPT_get_rvalue_LIST(expr), (WK_EXPR_CLBACK)dg_expr_uses, 
		    NULL, (Generic) r);
    dg_ip_query_add_ref(d, r, name, sym, callsite_id, callee_intrinsic,
			param_pos);  /* add info	*/
  }
}

/*-----------------------------------------------------------------------

	dg_ip_query_add_ref()	Use interprocedural info to build ref_list

*/

static void
dg_ip_query_add_ref(SymDescriptor d, Dg_ref_params *r, 
                    AST_INDEX expr, char *sym, int callsite_id, 
                    Boolean callee_intrinsic, unsigned int param_pos)
{
  fst_index_t si;      /* symbol id for sym in table */
  fst_index_t leader;  /* symbol id for equiv class leader in table */
  char  *leader_name;  /* name of equiv class leader */
  int offset;          /* offset of sym from equiv class leader */
  unsigned int length; /* size of sym */
  
  if (callee_intrinsic) {
    /* intrinsic functions always use, but never modify their arguments */
      dg_add_ref(r, expr, sym, T_IP_SCALAR | T_USE);	/* add use */
  } else {
    /* for an unknown function/subroutine, check IP info to see what 
     * (if any) type of access it performs on a particular argument
     */
    si = fst_QueryIndex(d, sym); 
    assert(fst_index_is_valid(si)); /* there must be an entry for sym */
    
    fst_Symbol_To_EquivLeader_Offset_Size(d, si, &leader, &offset, &length);
    if (length == FST_ADJUSTABLE_SIZE) length == INFINITE_INTERVAL_LENGTH;
    leader_name = (char *) fst_GetFieldByIndex(d, leader, SYMTAB_NAME);

    {
      Boolean callee_uses_actual_as_array = 
	IPQuery_CalleeParamIsAFormal(r->program_callgraph, r->current_proc, 
				     callsite_id, param_pos); 

      int ip_use_flag = (callee_uses_actual_as_array ? T_IP_WHOLE_ARRAY :
			 T_IP_SCALAR);

      if (r->program_callgraph != AST_NIL) {
	if (IPQuery_IsScalarMod(r->program_callgraph, r->current_proc, 
				callsite_id, sym, offset, length))
	  dg_add_ref(r, expr, sym, ip_use_flag | GUARDED | T_DEF); /* mod */
	
	if (IPQuery_IsScalarRef(r->program_callgraph, r->current_proc, 
				callsite_id, sym, offset, length))
	  dg_add_ref(r, expr, sym, ip_use_flag | T_USE);	/* use */
      } else {
	  dg_add_ref(r, expr, sym, T_IP_CONSERVATIVE | ip_use_flag | 
		     GUARDED | T_DEF); /* conservative mod */
      }
    }
  }
}

/*-----------------------------------------------------------------------

	dg_expr_uses()		Find all variables used in expression

	Returns:		Code for walk_expressions()

*/
static int
dg_expr_uses(AST_INDEX expr, Dg_ref_params *r)
{
  
  if (is_invocation(expr)) 
    {
      dg_invoc_refs(expr, r);
      return WALK_SKIP_CHILDREN;	
    } 
  else if (is_identifier(expr)) 
    {
      int i;
      char *sym = gen_get_text(expr);
      int index = fst_QueryIndex(r->sym_descriptor, sym);
    
      /* if sym is an ind. var. in any of the enclosing loops, continue walk */
      for (i = 0; i < r->loop_cur_level; i++) 
	{
	  /*
	   * char *sym2 = r->loops[r->loop_cur[i]].ivar_sym; 
	   * if STREQ(sym, sym2) return WALK_CONTINUE;
	   */
	  
	  if (index == r->loops[r->loop_cur[i]].ivar_index) 
	    return WALK_CONTINUE;
	}

      dg_add_ref(r, expr, sym, T_USE); /* add info */
    }

  return WALK_CONTINUE;	
}


/*-----------------------------------------------------------------------

	dg_add_ref()		Add reference to list

*/
void
dg_add_ref(Dg_ref_params *r, AST_INDEX node, char *sym, int def)
{
  Dg_ref_info       *RPtr;
  Dg_loop_info      *Loop;
  Dg_loop_var_info  *eqClass;
  SymDescriptor      d = r->sym_descriptor;
  int                level;
  int                i;
  Boolean            found;

  if (r->ref_num == r->ref_size)
    {
      r->ref_size <<= 1;
      r->refs = (Dg_ref_info *) reget_mem((void*)r->refs, 
					  sizeof(Dg_ref_info) * r->ref_size, 
					  "dg_add_ref");
      if (!r->refs)
	die_with_message("dg_add_ref(): out of memory\n");
    }

  /*------------------------------------------------------*/
  /* mark variable reference & store info about reference */

  RPtr = r->refs + r->ref_num++;

  RPtr->node  = node;
  RPtr->def   = def;
  RPtr->sym   = sym;

  RPtr->stmt  = r->stmts[r->stmt_num-1].node;
  RPtr->loop  = r->loops[r->loop_cur[r->loop_cur_level-1]].loop_hdr;

  RPtr->index = fst_QueryIndex(d, sym);
  RPtr->dims  = fst_GetFieldByIndex(d, RPtr->index, SYMTAB_NUM_DIMS);
                /* if we use dg_ast_dim here, it won't be consistent */

  fst_Symbol_To_EquivLeader_Offset_Size(d, RPtr->index, &(RPtr->leader), 
					&(RPtr->offset), (uint*)&(RPtr->size));
  if (RPtr->size == FST_ADJUSTABLE_SIZE)
    RPtr->size = INFINITE_INTERVAL_LENGTH;

  RPtr->elem_size = type_to_size_in_bytes[fst_GetFieldByIndex
					  (d, RPtr->index, SYMTAB_TYPE)];

  /* for all open loops insert this ref in its equiv. class list */
  for (level = 0; level < r->loop_cur_level; level++)
    {
      Loop = r->loops + r->loop_cur[level];
      
      found = false;
      /* linear search for the name - this list should be short */
      for (i = 0; i < Loop->loop_var_num; i++)
	if (RPtr->leader == Loop->loop_vars[i].leader)
	  { 
	    found = true;
	    break;
	  }
      
      if (found)     /* class had been referenced before, so its list exists */
	{
	  eqClass = &(Loop->loop_vars[i]);

	  if (eqClass->var_ref_num == eqClass->var_ref_size)
	    {
	      eqClass->var_ref_size <<= 1;
	      eqClass->var_refs = (int *)
		reget_mem((void*)eqClass->var_refs, 
			  sizeof(int) * eqClass->var_ref_size,
			  "dg_add_ref");
	      if (!eqClass->var_refs) 
		die_with_message("dg_add_ref(): out of memory\n");
	    }
	  
	  eqClass->var_refs[eqClass->var_ref_num++] = r->ref_num - 1;

	  /* now check if this exact name had appeared before */
	  found = false;
	  for (i = 0; i < eqClass->var_name_num; i++)
	    if (RPtr->index == (r->refs + eqClass->var_names[i])->index)
	      { 
		found = true;
		break;
	      }
	      
	  if (!found)     /* if name is found, we're done */
	    {
	      if (eqClass->var_name_num == eqClass->var_name_size)
		{
		  eqClass->var_name_size <<= 1;
		  eqClass->var_names = (int *)
		    reget_mem((void*)eqClass->var_names, 
			      sizeof(int) * eqClass->var_name_size,
			      "dg_add_ref");
		  if (!eqClass->var_names) 
		    die_with_message("dg_add_ref(): out of memory\n");
		}
	      
	      eqClass->var_names[eqClass->var_name_num++] = r->ref_num - 1;
	    }
	}
      
      else        /* the first reference to a var in this class in the loop */
	{
	  if (Loop->loop_var_num == Loop->loop_var_size)
	    {
	      Loop->loop_var_size <<= 1;
	      Loop->loop_vars  = (Dg_loop_var_info *) 
		reget_mem((void*)Loop->loop_vars, 
			  sizeof(Dg_loop_var_info) * Loop->loop_var_size,
			  "dg_add_ref");
	      if (!Loop->loop_vars) 
		die_with_message("dg_add_ref(): out of memory\n");
	    }
	  
	  eqClass = &(Loop->loop_vars[Loop->loop_var_num++]);
	  
	  eqClass->leader  = RPtr->leader;
	  eqClass->var_ref_num = 0;
	  eqClass->var_ref_size = R_LOOP_VAR_REFS;
	  eqClass->var_refs  = (int *) 
	    get_mem(sizeof(int) * R_LOOP_VAR_REFS, "dg_add_ref");
	  if (!eqClass->var_refs) 
	    die_with_message("dg_add_ref(): out of memory\n");
	  
	  eqClass->var_refs[eqClass->var_ref_num++] = r->ref_num - 1;

	  eqClass->var_name_num = 0;
	  eqClass->var_name_size = R_LOOP_VAR_NAMES;
	  eqClass->var_names  = (int *) 
	    get_mem(sizeof(int) * R_LOOP_VAR_NAMES, "dg_add_ref");
	  if (!eqClass->var_names) 
	    die_with_message("dg_add_ref(): out of memory\n");
	  
	  eqClass->var_names[eqClass->var_name_num++] = r->ref_num - 1;
	}
    }
}

/*-----------------------------------------------------------------------

	dg_build_DG()	Build edges for dependence graph from ref info

	Builds only dependences edges within loop nests
	Finds each loop nest, then works on it

*/
static void
dg_build_DG(Dg_ref_params *r)
{
  int i;
  int lvl;
  AST_INDEX stmt;
  
  /*--------------------------------------------------*/
  /* go through all loops found in each subprogram    */
  /* build dependences between pairs of refs	      */
  
  if (r->func_num > 0)      /* this means we are analyzing the whole module */
    {
      int	 f;

      for (f = 0; f < r->func_num; f++) 
	{
	  r->cfg_inst = r->funcs[f].cfg_inst;

	  for (i = r->funcs[f].first_loop; i < r->funcs[f].last_loop; i++) 
	    if (r->loops[i].loop_level == 1)  /* only top level loops */
	      dg_build_edges(r, &(r->loops[i]));
	}
    }
  
  else                    /* this means we are updating only a part of tree */
    {
      r->cfg_inst = NULL;

      for (i = 0; i < r->loop_num; i++)
	if (r->loops[i].loop_level == 1)     /* only top level loops */
	  dg_build_edges(r, &(r->loops[i]));
    }


  /*---------------------------------------*/
  /* build all statement level dependences */
  
  for (i = 0; i < r->stmt_num; i++) 
    {
      stmt = r->stmts[i].node;
    
      /*-----------------*/
      /* procedure calls */
    
      /* call edges only when no interprocedural information -- JMC 9/92 */
      if (is_call(stmt) && (r->program_callgraph == AST_NIL)) 
	{
	  for (lvl = r->stmts[i].level - 1; lvl > 0; lvl--)
	    dg_ins_edge(r->dg, r->dt, r->infoPtr, stmt, stmt, 
			dg_call, lvl, NULL, stmt, stmt);
	}
    
      /*----------------*/
      /* I/O statements	*/
    
      else if (io_stmt(stmt)) 
	{
	  for (lvl = r->stmts[i].level - 1; lvl > 0; lvl--)
	    dg_ins_edge(r->dg, r->dt, r->infoPtr, stmt, stmt, 
			dg_io, lvl, NULL, stmt, stmt);
	}
    }
}


/*-----------------------------------------------------------------

	dg_build_edges() 	New version

*/  
static void dg_build_edges(Dg_ref_params *r, Dg_loop_info *loop)
{
  int  i;
  int  last;
  int  src;
  int  sink;
  int *refs;
  Dg_ref_info *ref1;
  Dg_ref_info *ref2;

  /* go through the list of all vars referenced in the loop */

  if (!dg_get_input_dependences(r->dg)) 
    {
      for (i = 0; i < loop->loop_var_num; i++)
	{
	  last = loop->loop_vars[i].var_ref_num;
	  refs = loop->loop_vars[i].var_refs;

	  for (src = 0; src < last; src++)
	    {
	      ref1 = r->refs + refs[src];
	      if (ref1->def & T_USE)	 	/* ref1 must be a def */
		continue; 
      
	      for (sink = 0; sink <last; sink++) 
		{
		  ref2 = r->refs + refs[sink];
		  if (ref2->def & T_USE) 	/* true/anti dependence	*/
		    dg_build_pair(r, ref1, ref2,
				  ref1 < ref2 ? dg_true : dg_anti);

		  else if (ref1 <= ref2) /* output dep checked in 1 direction */
		    dg_build_pair(r, ref1, ref2, dg_output);
		}
	    }
	}
    } 
  else 
    {
      for (i = 0; i < loop->loop_var_num; i++)
	{
	  last = loop->loop_vars[i].var_ref_num;
	  refs = loop->loop_vars[i].var_refs;

	  for (src = 0; src < last; src++)
	    {
	      ref1 = r->refs + refs[src];
	      for (sink = 0; sink <last; sink++) 
		{
		  ref2 = r->refs + refs[sink];

		  if( ref1->def & T_USE )        
		    {	
		      if (ref2->def & T_USE && ref1 <= ref2) 
			/* input dependence, avoid duplicate testing */
			dg_build_pair(r, ref1, ref2, dg_input);
		    }

		  else /* ref1 is DEF */
		    {			
		      if (ref2->def & T_USE) 	/* true/anti dependence	*/
			dg_build_pair(r, ref1, ref2,
				      ref1 < ref2 ? dg_true:dg_anti);

		      else if (ref1 <= ref2) /* output dep checked in 1 direct */
			dg_build_pair(r, ref1, ref2, dg_output);
		    }
		}
	    }
	} 
    }
}

#ifdef OLD_CODE

/*-----------------------------------------------------------------------

	dg_build_edges()	Build edges for dependence graph from ref info

*/

static void
dg_build_edges(Dg_ref_params *r, int start, int end)
	/* int		 start;	index of first ref in loop */
	/* int		 end;	index of ref AFTER last ref in loop */
{
  Dg_ref_info *ref1;
  Dg_ref_info *ref2;
  Dg_ref_info *ref_last;
  
  /*------------------------------------*/
  /* 1) build all output dependences	*/
  
  /* defs are always provided in the following order:		*/
  
  /* 	(1,1), (1,2), ........................., (1,n), 	*/
  /*	       (2,2), (2,3), .................., (2,n),		*/
  /*	              (3,3), (3,4), ..........., (3,n),		*/
  /*	                      .................			*/
  /*	                                         (n,n)		*/
  
  /*--------------------------------------*/
  /* 2) build all true/anti dependences	*/
  
  /* def/uses are always provided with def first, 		*/
  /* dg_true if def occurs earlier, dg_anti otherwise 		*/
  /* this corresponds to the type of the loop indep dep, if any	*/
  
  /* 	(d1,u1), (d1,u2), ..., (d1,un), 	*/
  /*	(d2,u1), (d2,u2), ..., (d2,un),		*/
  /*	(d3,u1), (d3,u2), ..., (d3,un),		*/
  /*	...					*/
  /*	(dn,u1), (dn,u2), ..., (dn, un)		*/
  
  ref_last = r->refs + end;		/* don't use this ref	*/
  
  if(!dg_get_input_dependences(r->dg)) {
    for (ref1 = r->refs + start; ref1 < ref_last; ref1++) {
      if (ref1->def & T_USE)			/* ref1 must be a def	*/
	    continue; 
      
      for (ref2 = r->refs + start; ref2 < ref_last; ref2++) {
	    if (ref2->def & T_USE) {	/* true/anti dependence		*/
	      if (ref1->index == ref2->index) {    /* same var referenced */
	        dg_build_pair( r, ref1, ref2, ref1 < ref2 ? dg_true : dg_anti);
	      }
	    }
	    else if (ref2 >= ref1) {	/* output dep checked in 1 direction */
	      if (ref1->index == ref2->index)	    /* same var referenced */
	        dg_build_pair( r, ref1, ref2, dg_output);
	    }
      }
    }
  } else {
    for (ref1 = r->refs + start; ref1 < ref_last; ref1++) {
      for (ref2 = r->refs + start; ref2 < ref_last; ref2++) {
	    if( ref1->def & T_USE ) {	/* Input Dependence		*/
	      /* this will do both inter- and intra- statement edges */
	      if (ref2->def & T_USE) {		/* Input Dependence */
	        if (ref1->index == ref2->index) { /* same var referenced */
	          if( ref1 <= ref2 ) { /* avoid duplicate testing */
		        dg_build_pair( r, ref1, ref2, dg_input );
	          }
	        }
	      }
	    } else {			/* true/anti/output dependence	*/ 
	      if (ref2->def & T_USE) {		/* true/anti dependence	*/
	        if (ref1->index == ref2->index) { /* same var referenced */
	          dg_build_pair( r, ref1, ref2, ref1 < ref2 ? dg_true : dg_anti);
	        }
	      }
	      else if (ref2 >= ref1) {	/* output dep checked in 1 direction */
	        if (ref1->index == ref2->index) /* same var referenced */
	          dg_build_pair( r, ref1, ref2, dg_output);
	      }
	    }
      }
    } 
  }
}

#endif

/*-----------------------------------------------------------------------

	dg_build_pair()		Create DG edges for pair of refs (if needed)

	Takes dg_true/dg_anti/dg_output as type parameters.

 	if dg_output, ref1s = def, ref2s = def, AND ref1s preceeds ref2s
 	if dg_true,   ref1s = def, ref2s = use, AND ref1s preceeds ref2s
 	if dg_anti,   ref1s = def, ref2s = use, AND ref2s preceeds ref1s

*/

static void
dg_build_pair(Dg_ref_params *r, Dg_ref_info *ref1s, 
              Dg_ref_info *ref2s, DepType type)
{
  DG_Edge E,   *Edge;
  int 		i;
  int 		clevel;
  int 		distance;
  AST_INDEX	ref1 = ref1s->node;
  AST_INDEX	ref2 = ref2s->node;
  AST_INDEX  	stmt1;
  AST_INDEX 	stmt2;
  Boolean       imprecise = false;
  Boolean       hacking   = false;

  /*----------------------------------*/
  /* get actual dependence test info	*/
  
  Edge = &E;

  /* for actual arguments in the reference list because of scalar IP 
   * information, we fake the dependence analyzer into generating a 
   * scalar dependence by passing the AST_INDEX of the routine name 
   * rather than the AST_INDEX of the actual argument we will patch 
   * the dependence endpoint back to the actual after the dependence 
   * tester augments the edges with all of the additional information
   *
   * THIS IS SO UGLY!
   */
  
  if (ref1s->def & T_IP_WHOLE_ARRAY) {
    AST_INDEX invoc;
    for (invoc = ref1; !is_invocation(invoc); invoc = tree_out(invoc));
    Edge->src = gen_INVOCATION_get_name(invoc);
    hacking = true;
  } else Edge->src = ref1;
  
  if (ref2s->def & T_IP_WHOLE_ARRAY) {
    AST_INDEX invoc;
    for (invoc = ref2; !is_invocation(invoc); invoc = tree_out(invoc));
    Edge->sink = gen_INVOCATION_get_name(invoc);
    hacking = true;
  } else Edge->sink = ref2;

  /* don't add self-edges for conservative reference assumptions */ 
  if ((ref1s->def & T_IP_CONSERVATIVE) && (ref2s->def &  T_IP_CONSERVATIVE))
    return; 

  if (!hacking)
    {
      if (ref1s->index != ref2s->index)
	{
	  if (!dt_refs_overlap(ref1s->offset, ref1s->size, 
			       ref2s->offset, ref2s->size))
	    return;

	  if (ref1s->offset != ref2s->offset       ||
	      ref1s->elem_size != ref2s->elem_size ||
	      ref1s->dims != ref2s->dims           ||
	      ref1s->dims > 1)
	    imprecise = true;
	}
    }
      
  dt_test(r->dt, r->infoPtr, r->cfg_inst, Edge, 
	  ref1s->loop, ref2s->loop, imprecise);
  
  /* patch endpoints back to actual argument references, as necessary */
  if (ref1s->def & T_IP_WHOLE_ARRAY)  Edge->src = ref1;
  if (ref2s->def & T_IP_WHOLE_ARRAY)  Edge->sink = ref2;

  /*----------------------------------*/
  /* independence, no edges needed    */
  
  if (Edge->dt_type == DT_NONE)
    return;
  
  /*--------------------------------------------*/
  /* for interprocedural refs, add basic RSDs   */ 
  /* as descriptive strings for the refs        */

  if (ref1s->def & (T_IP_CONSERVATIVE | T_IP_WHOLE_ARRAY | T_IP_WHOLE_COMMON)){
    Edge->src_str =  
      dt_ssave(dg_conservative_rsd_string(r->ft, ref1s->node, ref1s->sym),
               r->dt);
  } else Edge->src_str = NULL;

  if (ref2s->def & (T_IP_CONSERVATIVE | T_IP_WHOLE_ARRAY | T_IP_WHOLE_COMMON)){
    Edge->sink_str =  
      dt_ssave(dg_conservative_rsd_string(r->ft, ref2s->node, ref2s->sym),
	       r->dt);
  } else Edge->sink_str = NULL;

  
  stmt1 = ref1s->stmt;
  stmt2 = ref2s->stmt;

  
  /*----------------------------------------------*/
  /* unknown dependence, put edges on all levels  */
  
  if (Edge->dt_type ==  DT_UNKNOWN) {		/* unknown	*/ 
    if( (type == dg_input) || (type == dg_output) ) {/* input & output deps */
      /* No self-referencing loop independent input dependencies	*/
      /* no loop independent output dependences for		*/
      /* single statement definitions!			*/
      
      if (ref1 != ref2)
	dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, -1, Edge, 
		    stmt1, stmt2);
      
      for (i = 1; i <= MAXLOOP; i++) {
	dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, i, Edge, stmt1, stmt2);
	
	if (ref1 != ref2)
	  /* added reverse JMC 2/93 */
	  dt_rev_info(r->dt, dg_ins_edge(r->dg, r->dt, r->infoPtr, 
					 ref2, ref1, type, i, Edge, stmt2, stmt1));
      }
    } else {			/* true/anti deps	*/ 
      /* no single statement loop independent	true dependences */
      /* single statement anti dependences only if memory analysis */
      
      if (stmt1 != stmt2 ||
	  (dg_get_input_dependences(r->dg) && type == dg_anti))
	dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, -1, Edge, stmt1, stmt2);
      
      for (i = 1; i <= MAXLOOP; i++) {
	dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, dg_true, i, Edge, stmt1, stmt2);
	dt_rev_info(r->dt, dg_ins_edge(r->dg, r->dt, r->infoPtr, 
				       ref2, ref1, dg_anti, i, Edge, stmt2, stmt1));
      }
    }
  }
  
  /*----------------------------------------------------------*/
  /* nonlinear dep, either symbolics or not perfectly nested	*/
  /* put edges on appropriate levels				*/
  
  else if (gen_is_dt_NONLIN(Edge)) {		/* nonlinear	*/
    clevel = gen_get_dt_CLVL(Edge);
    
    if( ((type == dg_input)) || (type == dg_output) ) {/* input & output deps*/
      /* No self-referencing loop independent input dependencies	*/
      /* no loop independent output dependences for		*/
      /* single statement definitions!			*/
      
      if (ref1 != ref2)
	dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, -1, Edge, stmt1, stmt2);
      
      for (i = 1; i <= clevel; i++)
	{
	  dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, i, Edge, stmt1, stmt2);
	  
	  if (ref1 != ref2)
	    /* added reverse JMC 2/93 */
	    dt_rev_info(r->dt, dg_ins_edge(r->dg, r->dt, r->infoPtr, 
					   ref2, ref1, type, i, Edge, stmt2, stmt1));
	}
    } else {			/* true/anti deps	*/ 
      /* no single statement loop independent	true dependences */
      /* single statement anti dependences only if memory analysis */
      
      if (stmt1 != stmt2 ||
	  (dg_get_input_dependences(r->dg) && type == dg_anti))
	dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, -1, Edge, stmt1, stmt2);
      
      for (i = 1; i <= clevel; i++) {
	dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, dg_true, i, Edge, stmt1, stmt2);
	dt_rev_info(r->dt, dg_ins_edge(r->dg, r->dt, r->infoPtr, ref2, 
				       ref1, dg_anti, i, Edge, stmt2, stmt1));
      }
    }
  }
  
  /*------------------------------------------------------*/
  /* found scalar dependence				  */
  
  else if (gen_is_dt_SCALAR(Edge)) {
    /* If two writes at same loop level, maybe loop independent 	*/
    /* dependence, but we need to look at control flow.  		*/
    
    /* DEBUG: just assume scalars act as kills for now  		*/
    /* i.e., no control flow around scalar ref				*/
    
    if (type == dg_input)
      return;          /* no scalar input dependences wanted */
    clevel = gen_get_dt_CLVL(Edge);
    
    if (type == dg_output) {		/*  output deps	*/
      /* no loop independent output dependences for	*/
      /* single statement definitions!			*/
      
      if (ref1 != ref2)
	dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, -1, Edge, stmt1, stmt2);

      /* SCALAR EDGES - ONLY AT LEVEL 0 !!!!! 
      dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, 0, Edge, stmt1, stmt2);
      */

      for (i = 1; i <= clevel; i++) {
	dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, i, Edge, stmt1, stmt2);
      }
    } else	{		/* true/anti deps	*/
      /* no single statement loop independent	true/anti dependences */
      
      if (stmt1 != stmt2) {
	if (type == dg_true)
	  dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, -1, Edge, stmt1, stmt2);
	else
	  /* added reverse JMC 2/93 */
	  dt_rev_info(r->dt, dg_ins_edge(r->dg, r->dt, r->infoPtr, 
					 ref2, ref1, type, -1, Edge, stmt2, stmt1));
      }

      /* SCALAR EDGES - ONLY AT LEVEL 0 !!!!! 
	dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, dg_true, 0, Edge, stmt1, stmt2);
	dt_rev_info(r->dt, dg_ins_edge(r->dg, r->dt, r->infoPtr, 
				       ref2, ref1, dg_anti, 0, Edge, stmt2, stmt1));
      */

      for (i = 1; i <= clevel; i++) {
	dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, dg_true, i, Edge, stmt1, stmt2);
	dt_rev_info(r->dt, dg_ins_edge(r->dg, r->dt, r->infoPtr, 
				       ref2, ref1, dg_anti, i, Edge, stmt2, stmt1));
	
	/* with more precise dataflow info, we can...		*/
	/* assume kill for LC true deps if def preceeds use	*/
	/* assume kill for LC antidep if use proceeds def	*/
	
	/*
	  if (type == dg_true)
	  dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, i, Edge);
	  else
	  dg_ins_edge(r->dg, r->dt, r->infoPtr, ref2, ref1, type, i, Edge);
	  */
      }
    }
  }
  
  /*------------------------------------------------------*/
  /* found dependence, put edges on appropriate levels	*/
  
  else {
    /*--------------------------------------*/
    /* check loop independent dependence	*/
    
    if (gen_is_dt_EQ(Edge)) {
      /* no single statement loop independent	*/
      /* true/output dependences 		*/
      /* single statement anti dependences if   */
      /* input flag is set for memory analysis  */
      
      if ((ref1 != ref2) && 
	  ((stmt1 != stmt2) ||
	   ((type == dg_anti) && dg_get_input_dependences(r->dg)))) {
	if( type == dg_output) 
	  dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, 
		      dg_output, -1, Edge, stmt1, stmt2);
	else if (type == dg_true)
	  dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, dg_true, -1, Edge, stmt1, stmt2);
	else if (type == dg_anti )
	  /* added reverse JMC 2/93 */
	  dt_rev_info(r->dt, dg_ins_edge(r->dg, r->dt, r->infoPtr, 
					 ref2, ref1, type, -1, Edge, stmt2, stmt1));
	else if (type == dg_input)
	  dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, -1, Edge, stmt1, stmt2);
      } else if((ref1 != ref2) && (type == dg_input) ) {	
	/* Append Intrastatement Input Edges	*/ 
	/* added reverse JMC 2/93 */
	dg_ins_edge(r->dg, r->dt, r->infoPtr, 
	            ref1, ref2, dg_input, -1, Edge, stmt1, stmt2);
      }
    }
    
    if (gen_is_dt_ALL_EQ(Edge))		/* no loop carried deps	*/
      return;						/* we're finished		*/
    
    /*--------------------------------------*/
    /* check loop carried dependences		*/
    
    clevel = gen_get_dt_CLVL(Edge);
    
    for (i = 1; i <= clevel; i++) {		/* find clevel of dep	*/
      distance = gen_get_dt_DIS(Edge, i);
      
      /*------------------------------*/
      /* if direction found			*/
      
      if (gen_is_dt_DIR(distance)) {
	/*--------------------------------------------------*/
	/* make sure only 1 edge for single stmt output dep	*/
	
	if (ref1 == ref2) {	/* self output and input dep */
	  if (type == dg_output) {
	    dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, 
			dg_output, i, Edge, stmt1, stmt2);
	   }
	  else
	    if (type == dg_input) {
	      dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, 
			  dg_input, i, Edge, stmt1, stmt2);
	     }
	}
	
	/*--------------------------------------*/
	/* check for possible true dependence	*/
	
	if ((ref1 != ref2) && 
	    ((distance != DDATA_GT) && (distance != DDATA_GE))) {
	  if( (type == dg_input) || (type == dg_output) )/* input & output deps	*/
	    dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, i, Edge, stmt1, stmt2);
	  else
	    dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, 
			dg_true, i, Edge, stmt1, stmt2);
	}
	
	/*--------------------------------------*/
	/* check for possible anti dependence	*/
	
	if ((ref1 != ref2) && 
	    ((distance != DDATA_LT) && (distance != DDATA_LE))) {
	  if( (type == dg_input) || (type == dg_output) ) {/* input & output deps	*/
	    dt_rev_info(r->dt, dg_ins_edge(r->dg, r->dt, r->infoPtr, 
					   ref2, ref1, type, i, Edge, stmt2, stmt1));
	  } else {
	    dt_rev_info(r->dt, dg_ins_edge(r->dg, r->dt, r->infoPtr, 
					   ref2, ref1, dg_anti, i, Edge, stmt2, stmt1));
	  }
	}
	
	/*----------------------------------------------------------*/
	/* if no loop indep possible at this level, then finished	*/
	
	if ((distance == DDATA_LT) || 
	    (distance == DDATA_GT) ||
	    (distance == DDATA_NE))
	  return; 
      }
      
      /*------------------------------*/
      /* else actual distance found	*/
      
      else if (distance > 0) {
	if( (type == dg_input) || (type == dg_output) )	/* input & output deps	*/
	  dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, type, i, Edge, stmt1, stmt2);
	else
	  dg_ins_edge(r->dg, r->dt, r->infoPtr, ref1, ref2, dg_true, i, Edge, stmt1, stmt2);
	return;
      } else if (distance < 0) {
	if( ((type == dg_input)) || (type == dg_output) )/* input & output deps	*/
	  dt_rev_info(r->dt, dg_ins_edge(r->dg, r->dt, r->infoPtr, 
					 ref2, ref1, type, i, Edge, stmt2, stmt1));
	else
	  dt_rev_info(r->dt, dg_ins_edge(r->dg, r->dt, r->infoPtr, ref2, 
					 ref1, dg_anti, i, Edge, stmt2, stmt1));
	return;
      }
    }
  }
}


/*-----------------------------------------------------------------------

	dg_ins_edge()		Insert edge into DG 

	Modeled after readgraph() in el/readgraph.c

*/

static DG_Edge *
dg_ins_edge(DG_Instance *dg, DT_info *dt, SideInfo *infoPtr, 
            AST_INDEX src, AST_INDEX sink, DepType type, int level,
            DG_Edge *Edge, AST_INDEX src_stmt, AST_INDEX sink_stmt)
{
  int           eindex;
  int           src_lvector;
  int		sink_lvector;
  int		src_ref;
  int		sink_ref;
  Boolean       stmt_only;
  DG_Edge      *EArray;
  DG_Edge      *Eptr;

  EArray = dg_get_edge_structure( dg );
  eindex = dg_alloc_edge( dg, &EArray);
  Eptr	 = EArray + eindex;

  src_lvector = sink_lvector = src_ref = sink_ref = -1;

  Eptr->type = type;
  Eptr->level = level;

  stmt_only = BOOL(type == dg_call || type == dg_exit || 
		   type == dg_control || type == dg_io);

  /*--------------------------------------------------------------*/
  /* sets src, sink, subscript info, src_stmt, sink_stmt fields	*/

  if (stmt_only)
    {
      Eptr->src = src_stmt;
      Eptr->sink = sink_stmt;
    }
  else
    {
      Eptr->src = src;
      Eptr->sink = sink;
    }

  /*-------------*/
  /* SOURCE stmt */

  if (!stmt_only)
    {
      /* does this stmt have a level vector? if not allocate one */

      if ((src_lvector = dg_get_info(  infoPtr, src_stmt, type_levelv)) == -1)
	{
	  src_lvector = dg_alloc_level_vector( dg, MAXLOOP);
	  dg_put_info( infoPtr, src_stmt, type_levelv, src_lvector);
	}
      Eptr->src_vec = src_lvector;

      /* does this variable have a reference list? if not allocate one */

      if ((src_ref = dg_get_info( infoPtr, src, type_levelv)) == -1)
	{
	  src_ref = dg_alloc_ref_list( dg );
	  dg_put_info( infoPtr, src, type_levelv, src_ref);
	}
      Eptr->src_ref = src_ref;
    }

  /*-----------*/
  /* SINK stmt */

  /* does this stmt have a level vector? if not allocate one */

  if ((sink_lvector = dg_get_info( infoPtr, sink_stmt, type_levelv)) == -1)
    {
      sink_lvector = dg_alloc_level_vector( dg, MAXLOOP);
      dg_put_info( infoPtr, sink_stmt, type_levelv, sink_lvector);
    }
  Eptr->sink_vec = sink_lvector;

  if (stmt_only)
    {
      Eptr->src_vec = Eptr->sink_vec;
      Eptr->src_ref = -1;
      Eptr->sink_ref = -1;
    }
  else
    {
      /* does this variable have a reference list? if not allocate one */

      if ((sink_ref = dg_get_info( infoPtr, sink, type_levelv)) == -1)
	{
	  sink_ref = dg_alloc_ref_list( dg );
	  dg_put_info( infoPtr, sink, type_levelv, sink_ref);
	}
      Eptr->sink_ref = sink_ref;
    }

  /* if Edge == NULL, then must be statement level dependence */

  if (Edge) {
    dt_copy_info(dt, Edge, Eptr);
    Eptr->consistent = Edge->consistent;
    Eptr->symbolic   = Edge->symbolic;
  } else {
    Eptr->dt_type = DT_UNKNOWN;
  }


  /* fake some RSDs (what a kludge!) */
  if (Edge) {
    Eptr->src_str = Edge->src_str;
    Eptr->sink_str = Edge->sink_str;
  } else {
    Eptr->src_str = NULL;
    Eptr->sink_str = NULL;
  }


  if (Eptr->level > MAXLOOP)
    {
      message("Unless lvector realloc fixed... we are doomed.");
    }

  /*--------------------------*/
  /* check interchange status */

  /*--------------------------------------------------*/
  /* statement level dependences inhibit everything	*/

  if (stmt_only)
    {
      Eptr->ic_preventing = true;
      Eptr->ic_sensitive = false;
    }

  /*------------------------------------------------------------------*/
  /* not statement level dependence, need to actually check dep		*/

  else
    {
      dt_set_intchg(Eptr, level);
    }

  /*---------------------------------*/
  /* now add to the dependence graph */

  dg_add_edge( dg, eindex);

  return Eptr;
}


/*-----------------------------------------------------------------------

	dg_build_LI()	Build edges for dependence graph from ref info

	Modeled after read_index() in el/readgraph.c

*/
static void
dg_build_LI(Dg_ref_params *r, FortTree ft)
{
  int 			 i, j, k;
  Slist			*sptr;
  Slist			*save_sh; /* last elem of shared list  */
  Slist			*save_p;  /* last elem of private list */
  Loop_info		*saveLptr;
  LI_Instance		*LI;
  Loop_info		*Lptr;
  Dg_loop_info   	*Loop;
  Boolean		 found;
  Dg_ref_info		*ref;

  /*---------------------------------------------*/
  /* create index table for ref relative to uses */

  LI = (LI_Instance *) r->li;


  LI->Linfo = NULL;
  LI->cur_loop = NULL;
  LI->num_loops = 0;
  Lptr = NULL;

  /*-------------------------------------------*/
  /* go through all loops found in the program */

  for (i = 0, Loop = r->loops; i < r->loop_num; i++, Loop++)
    {
      /*-----------------------------------*/
      /* allocate and initialize Loop_info */

      saveLptr = Lptr;
      Lptr = (Loop_info *) get_mem(sizeof(Loop_info), "dg_build_LI");

      Lptr->loop_hdr_index = Loop->loop_hdr;
      Lptr->loop_level = Loop->loop_level;
      Lptr->parallelized = false;
      Lptr->shvar_list = NULL;
      Lptr->pvar_list = NULL;
      Lptr->next = NULL;
      Lptr->prev = saveLptr;
      Lptr->cflow = NOFLOW;
      Lptr->ndeps = 0;

      if (!saveLptr)		/* append to list of loops */
	{
	  LI->Linfo = Lptr;
	  LI->cur_loop = NULL;
	}
      else
	saveLptr->next = Lptr;

      LI->num_loops++;

      /*---------------------------------------------------*/
      /* add index var for loop into private variable list */
		
      save_p = li_create_Slist();
      li_init_Slist(save_p, Loop->ivar_sym, "_local_", 0);
      Lptr->pvar_list = save_p;

      /*-------------------------------------------------*/
      /* add all ivars of enclosed loops to private list */

      j = i+1;
      while ((j < r->loop_num) && 
	     (r->loops[j].loop_level > Loop->loop_level))
	{
	  /*--------------------------------------------*/
	  /* make sure ivar not already labeled private */
	  /* look in private var list			*/

	  found = false;
	  sptr = Lptr->pvar_list;
	  while (NOT(found) && sptr)
	    {
	      if (STREQ(sptr->name, r->loops[j].ivar_sym))
		found = true;
	      else
		sptr = sptr->next;
	    }

	  if (NOT(found))
	    {
	      save_p->next = li_create_Slist();
	      li_init_Slist(save_p->next, r->loops[j].ivar_sym, "_local_", 0);
	      save_p = save_p->next;
	    }

	  j++;
	}

      /*------------------------------------------*/
      /* Look at all variables referenced in loop */

      for (j = 0; j < Loop->loop_var_num; j++)
	{
	  for (k = 0; k < Loop->loop_vars[j].var_name_num; k++)
	    {
	      ref = r->refs + Loop->loop_vars[j].var_names[k];

	      sptr = li_create_Slist();
	      li_init_Slist(sptr, ref->sym, "_local_", ref->dims); 

	      /*----------------------------------------------------*/
	      /* find out where variable is ref'd before/after loop */

	      dt_refs_outside(ref, Loop, r, sptr);

	      /*----------------------------------------------------*/
	      /* check whether variable should be private or shared */

	      /* variables may be made private if it has     */
	      /*  1) no reaching uses after the loop         */
	      /*  2) no reaching definitions before the loop */
	      /*  3) no loop-carried TRUE dependences	 */

	      if ((sptr->use_after == AST_NIL) && 
		  (ref->dims == 0) &&   /* assume that all arrays are shared */
		  !dt_reached_use(ref, Loop, r, ft))
		{
		  sptr->why	= (VarType)4;	/* private variable */
		  if (!Lptr->pvar_list)
		    Lptr->pvar_list = sptr;
		  else
		    save_p->next = sptr;
		  save_p = sptr;
		}

	      /*---------------------------------------------------------*/
	      /* variable failed the test, add it to the shared var list */

	      else
		{
		  if (!Lptr->shvar_list)
		    Lptr->shvar_list = sptr;
		  else
		    save_sh->next = sptr;
		  save_sh = sptr;
		}
	    }
	}
    }
}		


/*-----------------------------------------------------------------------

	dt_refs_outside()	finds def preceeding loop, use after loop,
				for a given variable

*/
static void
dt_refs_outside(Dg_ref_info *vref, Dg_loop_info *Loop, 
                Dg_ref_params *r, Slist *sptr)
     /* Dg_ref_info 	*vref;	variable being analyzed */
     /* Dg_loop_info   	*Loop;	loop it's in		*/
     /* Dg_ref_params	*r;	info on other variables */
     /* Slist		*sptr;	LI variable info	*/
     
{
  int 		 k;
  Dg_ref_info 	*ref;	  
  int            leader = vref->leader;
  int            index  = vref->index;
  Boolean        scalar_ref = (Boolean)(vref->dims == 0);

  /*--------------------------------------*/
  /* default: set to AST_NIL if not found */

  sptr->def_before = AST_NIL;	
  sptr->use_after = AST_NIL;

  /*-----------------------------------------*/
  /* look for closest definition to the loop */

  /* should be checking for reaching definitions :-( */

  for (k = Loop->loop_start - 1, ref = r->refs + k; 
       k >= 0; k--, ref--)
    {
      if ((leader == ref->leader) && /* found variable */
	  ((index == ref->index) || 
	   dt_refs_overlap(vref->offset,vref->size,ref->offset,ref->size))  &&
	  (ref->def & T_DEF))	/* and is definition */
	{
	  sptr->def_before = ref->node;
	  break;
	}
    }

  /*-------------------------------------*/
  /* look for closest use after the loop */

  /* should be checking for reaching uses :-(	*/

  /* will approximate for now, but need control flow info	*/

  for (k = Loop->loop_end, ref = r->refs + k; 
       k < r->ref_num; k++, ref++)
    {
      if (leader == ref->leader &&
	  ((index == ref->index) || 
	   dt_refs_overlap(vref->offset, vref->size, ref->offset, ref->size)))
	{
	  /* if found use, finished */
	  if (ref->def & T_USE)		
	    {
	      sptr->use_after = ref->node;
	      return;
	    }

	  /* if unguarded DEF & scalar, then no reaching use */
	  /* so stop and return AST_NIL immediately	     */

	  else if (scalar_ref && 
		   (ref->def & T_DEF) && !(ref->def & T_GUARDED) &&
		   (index == ref->index)) /* for eqiv. vars we're not sure*/
	    {
	      sptr->use_after = AST_NIL;
	      return;		/* unguarded DEF is a kill for scalars	*/
	    }

	  /* if guarded def, inconclusive whether use exists */
	}
    }
}


/*-----------------------------------------------------------------------

	dt_reached_use() - finds whether DEF from outside loop or
			   previous iteration reaches USE of var in loop

*/
static Boolean
dt_reached_use(Dg_ref_info *vref, Dg_loop_info *Loop, 
               Dg_ref_params *r, FortTree ft)
{
  int 		     i;
  Dg_loop_var_info  *eqClass;
  Dg_ref_info 	    *ref;		/* variable being analyzed	*/
  int  		     leader = vref->leader; 
  int                index  = vref->index;
  int                last   = Loop->loop_var_num;

  /*-------------------------------------------*/
  /* look for first USE occurence in loop body */
  
  for (i = 0, eqClass = Loop->loop_vars; i < last; i++, eqClass++)
    {
      if (leader == eqClass->leader)
        {
	  for (i = 0; i < eqClass->var_ref_num; i++)
	    {
	      ref = r->refs + eqClass->var_refs[i]; 
	      if ((index == ref->index) ||
		  dt_refs_overlap(vref->offset, vref->size, 
				  ref->offset, ref->size))
		{
		  if ((ref->def & T_USE) || (ref->def & T_IP_CONSERVATIVE))
		    return true;
	  
		  if (/* scalar_ref && */
		      index == ref->index && 
		      (ref->def & T_DEF) && !(ref->def & T_GUARDED))
		    {
		      return false;
		      /* if array, must apply array kill analysis */
		      /* for now just assume not killed */
		    }
		}
	    }
	}
    }
  
  return true;  /* no USE encountered in loop */
                /* possible interprocedural USE? tseng - Apr 93 */
}



/*-----------------------------------------------------------------------

	dt_rev_info()		Reverse Dt info for DG Edge in place

*/
static void
dt_rev_info(DT_info *dt, DG_Edge *Edge)
{
  int i;
  int max_level;
  int dat;

  /*--------------*/
  /* reverse data */

  max_level = gen_get_dt_CLVL(Edge);

  for (i = 0; i < max_level; i++)
    {
      dat = Edge->dt_data[i];

      if (gen_is_dt_DIR(dat))
	{
	  switch (dat)
	    {
	    case DDATA_LT:
	      Edge->dt_data[i] = DDATA_GT;
	      break;

	    case DDATA_GT:
	      Edge->dt_data[i] = DDATA_LT;
	      break;

	    case DDATA_LE:
	      Edge->dt_data[i] = DDATA_GE;
	      break;

	    case DDATA_GE:
	      Edge->dt_data[i] = DDATA_LE;
	      break;

	    default:
	      /* case DDATA_NE, DDATA_ANY, DDATA_ERROR	*/
	      break;
	    }
	}
      else
	{
	  Edge->dt_data[i] = -dat;
	}
    }

  /*---------------------------*/
  /* swap src and sink strings */
  {
    char *tmp = Edge->src_str; 
    Edge->src_str = Edge->sink_str;
    Edge->sink_str = tmp;
  }

  /*------------------------------------*/
  /* generate string from reversed data	*/

  dt_info_str(dt, Edge);	

  /*-------------------------------------------------------------*/
  /* redo the interchange bits using the new distance/dir vector */

  dt_set_intchg(Edge, Edge->level);
}


/*-----------------------------------------------------------------------

	dg_clear()	- free as much of the DG stuff as convenient

*/

static void
dg_clear(DG_Instance *dg, DT_info *dt, SideInfo *infoPtr, 
         AST_INDEX root)
{
	Clear_edge_parm params;
	Clear_lv_parm parm;

	/*------------------*/
	/* clear DG edges	*/

	params.dg	= dg;
	params.dt	= dt;
	params.infoPtr	= infoPtr;
	params.Earray	= dg_get_edge_structure(dg);

	walk_statements(root, LEVEL1, (WK_STMT_CLBACK)dt_walk_clear_edges, NULL, 
			(Generic) &params);

	/*------------------*/
	/* now zap level vectors, too	*/

	/* Used to clean up after tree_copy(), which  */
	/* copies all side array values to the new tree. */

	parm.infoPtr	= infoPtr;
	/* The following line was commented out because the new */
	/* trees do not support the Ast_side_array structure and */
	/* because Chau-Wen claimed this was a useless line -- dcs 3/9/92 */
	/*	parm.info_side_array = 
	 *	((Ast_side_array *) PED_INFO_SIDE_ARRAY(ped))->array;
	 */

	walk_expression(root, (WK_EXPR_CLBACK)dt_walk_clear_lv, NULL, (Generic) &parm);

	/*------------------*/

	dt_update_loopref( dt, infoPtr, root);	/* then update loop/ref info */
}


/*-----------------------------------------------------------------------

	

*/

static int
dt_walk_clear_lv(AST_INDEX expr, Clear_lv_parm *parm)
{
	int vector;

	if ((vector = dg_get_info(parm->infoPtr, expr, type_levelv)) != NO_LEVELV)
		dg_put_info(parm->infoPtr, expr, type_levelv, UNUSED);

	return WALK_CONTINUE;
}


/*-----------------------------------------------------------------------

	dt_walk_clear_edges()	- free all DG edges for a statement

*/


static int
dt_walk_clear_edges(AST_INDEX stmt, int level, 
                    Clear_edge_parm *params)
{
	EDGE_INDEX	  idx_old;		/* index of current dependence edge	*/
	EDGE_INDEX	  idx_new;		/* index of next dependence edge	*/
	int				Lvec;		/* level vector						*/
	int				nest;

	DG_Edge		*Earray = params->Earray;

	if ((Lvec = dg_get_info(params->infoPtr, stmt, type_levelv)) != -1)
	{
		nest = dg_length_level_vector(params->dg, Lvec);

		for (level = LOOP_INDEPENDENT; level <= nest; level++)
		{
			idx_old = dg_first_src_stmt(params->dg, Lvec, level);
			if (idx_old != -1)
			{
				/* make sure we get the successor BEFORE we delete edge	*/

				while ((idx_new = dg_next_src_stmt(params->dg, idx_old)) != -1)
				{
					dg_delete_edge(params->dg, idx_old);
					dg_free_edge(params->dg, Earray, idx_old);

					idx_old = idx_new;		/* now go to successor	*/

				}

				dg_delete_edge(params->dg, idx_old);
				dg_free_edge(params->dg, Earray, idx_old);
			}
		}
	}

	return WALK_CONTINUE;
}


/*----------------------------------------------------------------------

	dt_refs_overlap()	checks whether two references overlap

*/
static Boolean
dt_refs_overlap(int offset1, int size1, int offset2, int size2)
{
  if (size1 != INFINITE_INTERVAL_LENGTH)
    {
      if (offset2 >= offset1 + size1)
	return false;
      if (offset2 >= offset1)
	return true;
      if (size2 != INFINITE_INTERVAL_LENGTH)
	return (Boolean)(offset2 + size2 > offset1);
      else
	return true;
    }
  else if (size2 != INFINITE_INTERVAL_LENGTH)
    return (Boolean)(offset2 + size2 > offset1);

  else 
    return true;
}      


#ifdef UNUSED_CODE
/*-----------------------------------------------------------------------

	dg_show_refs()		Print out list of refs found
						Diagnostic only

*/

static void
dg_show_refs(Dg_ref_params *r)
{
	int i;

	printf("----------\nRefs Found:\n");
	for (i = 0; i < r->ref_num; i++)
	{
		printf("(%2d) %s: %s (AST = %d)\n", i, 
			r->refs[i].use ? "Use" : "Def", 
			r->refs[i].sym, r->refs[i].node);
	}

	printf("----------\nLoops Found:\n");
	for (i = 0; i < r->loop_num; i++)
	{
		printf("(%2d) Loop AST = %d (IV = %s, Level = %d) [%d:%d]\n", i,
			r->loops[i].loop_hdr, r->loops[i].ivar_sym, 
			r->loops[i].loop_level, r->loops[i].loop_start,  
			r->loops[i].loop_end);
	}

}
#endif	/* UNUSED_CODE */


/*--------------------------------------------------------------------	*/
/*	Field Access Functions for the Dg_ref_info structure		*/
/*--------------------------------------------------------------------	*/
AST_INDEX
dg_ref_info_node(Dg_ref_info *ref_info)
{
  return	(ref_info->node);
}

char *
dg_ref_info_sym(Dg_ref_info *ref_info)
{
  return	(ref_info->sym);
}

int
dg_ref_info_def(Dg_ref_info *ref_info)
{
  return	(ref_info->def);
}

