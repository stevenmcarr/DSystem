/* $Id: scalar.C,v 1.29 1996/01/15 10:23:02 carr Exp $ */

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <general.h>
#include <sr.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <scalar.h>
#include <LoopStats.h>
#include <fort/AstIter.h>

#ifndef check_h
#include <check.h>
#endif

#ifndef codegen_h
#include <codegen.h>
#endif

#ifndef dfantic_h
#include <dfantic.h>
#endif

#ifndef dfavail_h
#include <dfavail.h>
#endif

#ifndef dfrgen_h
#include <dfrgen.h>
#endif

#ifndef gavail_h
#include <gavail.h>
#endif

#ifndef insert_h
#include <insert.h>
#endif

#ifndef name_h
#include <name.h>
#endif

#ifndef moderate_h
#include <moderate.h>
#endif

#ifndef pick_h
#include <pick.h>
#endif

#ifndef profit_h
#include <profit.h>
#endif

#ifndef prune_h
#include <prune.h>
#endif

#ifndef table_h
#include <table.h>
#endif

#ifndef gi_h
#include <fort/gi.h>
#endif

#ifndef mh_config_h
#include <mh_config.h>
#endif

#include <malloc.h>
#include <mem_util.h>
#include <pt_util.h>

int dummy = 0; /* this decl keeps Rn from dying in get_mem (why?) */


Boolean IsIndexUniform(AST_INDEX node, 
		       AST_INDEX expr,
		       SymDescriptor symtab)
  {
   Boolean linear;
   int coeff,index;

     if (is_identifier(node))
       {
	pt_get_coeff(expr,gen_get_text(node),&linear,&coeff);
	return (linear);
       }
     return(true);
  }
   

Boolean IsSubscriptUniform(AST_INDEX node,
			   SymDescriptor symtab)
			  

  {
   AST_INDEX sublist,sub,Inode;
   Boolean Uniform = true;

     sublist = gen_SUBSCRIPT_get_rvalue_LIST(node);
     for (sub = list_first(sublist);
	  sub != AST_NIL && Uniform;
	  sub = list_next(sub))
       for (AstIter AIter(sub,false);(Inode = AIter()) != AST_NIL && Uniform;)
	 Uniform = BOOL(Uniform && IsIndexUniform(Inode,sub,symtab));
     return(Uniform);
  }
       

static int count_arrays(AST_INDEX          node,
			prelim_info_type   *prelim_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX  name;
   EDGE_INDEX edge,
              next_edge;
   DG_Edge    *dg;
   int        vector,
              refs;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	get_scalar_info_ptr(name)->generator = -1;
	get_scalar_info_ptr(name)->surrounding_do=prelim_info->surrounding_do;
	get_scalar_info_ptr(name)->array_num = prelim_info->array_refs++;
	dg = dg_get_edge_structure( PED_DG(prelim_info->ped));
	vector = get_info(prelim_info->ped,name,type_levelv);
	for (edge = dg_first_src_ref( PED_DG(prelim_info->ped),vector);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_ref( PED_DG(prelim_info->ped),edge);
	   if ((get_scalar_info_ptr(dg[edge].sink) == NULL) ||
	       (dg[edge].level != prelim_info->level &&
		dg[edge].level != LOOP_INDEPENDENT))
	     dg_delete_free_edge( PED_DG(prelim_info->ped),edge);
	  }
	for (edge = dg_first_sink_ref( PED_DG(prelim_info->ped),vector);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_sink_ref( PED_DG(prelim_info->ped),edge);
	   if ((get_scalar_info_ptr(dg[edge].src) == NULL) ||
	       (dg[edge].level != prelim_info->level &&
		dg[edge].level != LOOP_INDEPENDENT))
	     dg_delete_free_edge( PED_DG(prelim_info->ped),edge);
	  }
	if (IsSubscriptUniform(node,prelim_info->symtab))
	  prelim_info->UniformRefs++;
	else
	  prelim_info->NonUniformRefs++;
       }
     else
       if (is_identifier(node) && !is_subscript(tree_out(node)) && 
	   !is_invocation(tree_out(node)))
	 {
	  if ((refs=fst_GetField(prelim_info->symtab,gen_get_text(node),REFS))
	      == 0)
	    {
	     if (gen_get_converted_type(node) == TYPE_REAL)
	       prelim_info->scalar_regs++;
	     else if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION)
	       prelim_info->scalar_regs += 
	         ((config_type *)PED_MH_CONFIG(prelim_info->ped))->double_regs;
	     else if (gen_get_converted_type(node) == TYPE_COMPLEX)
	       prelim_info->scalar_regs += 2;
	    }
	  fst_PutField(prelim_info->symtab,(int)gen_get_text(node),REFS,++refs);
	 }
     return(WALK_CONTINUE);
  }

static int get_prelim_info(AST_INDEX          stmt,
			   int                level,
			   prelim_info_type   *prelim_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX label;

     create_stmt_info_ptr(stmt,prelim_info->ar);
     get_stmt_info_ptr(stmt)->stmt_num = prelim_info->stmt_num++;
     get_stmt_info_ptr(stmt)->generated = false;
     if (is_comment(stmt))
       return(WALK_CONTINUE);
     if ((label = gen_get_label(stmt)) != AST_NIL)
       {
	prelim_info->contains_goto_or_label = true;
	set_label_sym_index(label,fst_QueryIndex(prelim_info->symtab,
						 gen_get_text(label)));
	fst_PutFieldByIndex(prelim_info->symtab,get_label_sym_index(label),
			    LBL_STMT,stmt);
       }
     if (is_assignment(stmt))
       {
	walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),
			(WK_EXPR_CLBACK)count_arrays,NOFUNC,(Generic)prelim_info);
	walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),
			(WK_EXPR_CLBACK)count_arrays,NOFUNC,(Generic)prelim_info);
       }
     else if (is_guard(stmt))
       {
	prelim_info->contains_cf = true;
	walk_expression(gen_GUARD_get_rvalue(stmt),(WK_EXPR_CLBACK)count_arrays,
			NOFUNC,(Generic)prelim_info);
       }
     else if (is_logical_if(stmt))
       {
	prelim_info->contains_cf = true;
	walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),
			(WK_EXPR_CLBACK)count_arrays,NOFUNC,(Generic)prelim_info);
       }
     else if (is_write(stmt))
       walk_expression(gen_WRITE_get_data_vars_LIST(stmt),
		       (WK_EXPR_CLBACK)count_arrays,NOFUNC,
		       (Generic)prelim_info);
     else if (is_print(stmt))
       walk_expression(gen_PRINT_get_data_vars_LIST(stmt),
		       (WK_EXPR_CLBACK)count_arrays,NOFUNC,
		       (Generic)prelim_info);
     else if (is_read_short(stmt))
       walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),
		       (WK_EXPR_CLBACK)count_arrays,NOFUNC,(Generic)prelim_info);
     else if (is_arithmetic_if(stmt))
       {
	prelim_info->contains_cf = true;
	prelim_info->contains_goto_or_label = true;
	walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),
			(WK_EXPR_CLBACK)count_arrays,NOFUNC,(Generic)prelim_info);
       }
     else if (is_if(stmt))
       prelim_info->contains_cf = true; 
     else if (is_goto(stmt) || is_computed_goto(stmt))
       {
	prelim_info->contains_cf = true;
	prelim_info->contains_goto_or_label = true;
       }
     else if (is_call(stmt))
       walk_expression(gen_CALL_get_invocation(stmt),(WK_EXPR_CLBACK)count_arrays,
		       NOFUNC,(Generic)prelim_info);
     return(WALK_CONTINUE);
  }

static int check_gotos(AST_INDEX        stmt,
		       int              level,
		       prelim_info_type *prelim_info)

  {
   AST_INDEX   label,
               lbl_stmt1,
               lbl_stmt2,
               lbl_stmt3;
   int         stmt_num;
   fst_index_t index1,
               index2,
               index3;
   


     if (is_goto(stmt))
       {
	index1 = fst_QueryIndex(prelim_info->symtab,
				gen_get_text(gen_GOTO_get_lbl_ref(stmt)));
	if ((lbl_stmt1 = fst_GetFieldByIndex(prelim_info->symtab,index1,
					     LBL_STMT)) == AST_NIL)
	  {
	   prelim_info->jumps_ok = false;
	   prelim_info->premature_exit = true;
	   return(WALK_ABORT);
	  }
	else if (get_stmt_info_ptr(stmt)->stmt_num >=
		get_stmt_info_ptr(lbl_stmt1)->stmt_num)
	  {
	   prelim_info->jumps_ok = false;
	   prelim_info->backjump = true;
	   return(WALK_ABORT);
	  }
	else
	  {
	   set_label_sym_index(gen_GOTO_get_lbl_ref(stmt),index1);
	   fst_PutFieldByIndex(prelim_info->symtab,index1,REFS,
			       (int)fst_GetFieldByIndex(prelim_info->symtab,
							index1,REFS)+1);     
	  }
       }
     else if (is_arithmetic_if(stmt))
       {
	index1 = fst_QueryIndex(prelim_info->symtab,gen_get_text(
                                gen_ARITHMETIC_IF_get_lbl_ref1(stmt)));
	index2 = fst_QueryIndex(prelim_info->symtab,gen_get_text(
                                gen_ARITHMETIC_IF_get_lbl_ref2(stmt)));
	index3 = fst_QueryIndex(prelim_info->symtab,gen_get_text(
                                gen_ARITHMETIC_IF_get_lbl_ref3(stmt)));
	if ((lbl_stmt1 = fst_GetFieldByIndex(prelim_info->symtab,index1,
					     LBL_STMT)) == AST_NIL ||
	    (lbl_stmt2 = fst_GetFieldByIndex(prelim_info->symtab,index2,
					     LBL_STMT)) == AST_NIL ||
	    (lbl_stmt3 = fst_GetFieldByIndex(prelim_info->symtab,index3,
					     LBL_STMT)) == AST_NIL)
	 {
	  prelim_info->jumps_ok = false;
	  prelim_info->premature_exit = true;
	  return(WALK_ABORT);
	 }
       else if (get_stmt_info_ptr(stmt)->stmt_num >=
		get_stmt_info_ptr(lbl_stmt1)->stmt_num 
                || get_stmt_info_ptr(stmt)->stmt_num >=
		get_stmt_info_ptr(lbl_stmt2)->stmt_num 
                || get_stmt_info_ptr(stmt)->stmt_num >=
		get_stmt_info_ptr(lbl_stmt3)->stmt_num)
	 {
	  prelim_info->jumps_ok = false;
	  prelim_info->backjump = true;
	  return(WALK_ABORT);
	 }
       else
	 {
	  set_label_sym_index(gen_ARITHMETIC_IF_get_lbl_ref1(stmt),index1);
          fst_PutFieldByIndex(prelim_info->symtab,index1,REFS,
			      (int)fst_GetFieldByIndex(prelim_info->symtab,
							 index1,REFS)+1);     
	  set_label_sym_index(gen_ARITHMETIC_IF_get_lbl_ref2(stmt),index2);
          fst_PutFieldByIndex(prelim_info->symtab,index2,REFS,
			      (int)fst_GetFieldByIndex(prelim_info->symtab,
							 index2,REFS)+1);     
	  set_label_sym_index(gen_ARITHMETIC_IF_get_lbl_ref3(stmt),index3);
          fst_PutFieldByIndex(prelim_info->symtab,index3,REFS,
			      (int)fst_GetFieldByIndex(prelim_info->symtab,
							 index3,REFS)+1);     
	 }
       }
     else if (is_computed_goto(stmt))
       {
	stmt_num = get_stmt_info_ptr(stmt)->stmt_num;
	for (label = list_first(gen_COMPUTED_GOTO_get_lbl_ref_LIST(stmt));
	     label != AST_NIL;
	     label = list_next(label))
	  {
	   index1 = fst_QueryIndex(prelim_info->symtab,gen_get_text(label));
	   if ((lbl_stmt1 = fst_GetFieldByIndex(prelim_info->symtab,index1,
						LBL_STMT)) == AST_NIL)
	     {
	      prelim_info->jumps_ok = false;
	      prelim_info->premature_exit = true;
	      return(WALK_ABORT);
	     }
	   else if (stmt_num >= get_stmt_info_ptr(lbl_stmt1)->stmt_num)
	     {
	      prelim_info->jumps_ok = false;
	      prelim_info->backjump = true;
	      return(WALK_ABORT);
	     }
	   else
	     {
	      set_label_sym_index(label,index1);
	      fst_PutFieldByIndex(prelim_info->symtab,index1,REFS,
				  (int)fst_GetFieldByIndex(prelim_info->symtab,
							   index1,REFS)+1);
	     }
	  }
       }
     return(WALK_CONTINUE);
  }


static int check_illegal_jumps (AST_INDEX        stmt,
				int              level,
				prelim_info_type *prelim_info)

  {
   AST_INDEX label;
   fst_index_t index1;

     if ((label = gen_get_label(stmt)) != AST_NIL)
       {		       
	index1 = fst_QueryIndex(prelim_info->symtab,gen_get_text(label));
	if (fst_GetFieldByIndex(prelim_info->symtab,index1,REFS) == 0)
	  {
	   prelim_info->illegal_jump = true;
	   prelim_info->jumps_ok = false;
	   return(WALK_ABORT);
	  }
       }
     return(WALK_CONTINUE);
  }
	

static int get_expr_regs(AST_INDEX     node,
			 reg_info_type *reg_info)

  {
   int label1,label2;
   int nregs;
   scalar_info_type *sptr;
  
     if (gen_get_converted_type(node) == TYPE_REAL)
       nregs = 1;
     else if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION)
       nregs = reg_info->config->double_regs;
     else if (gen_get_converted_type(node) == TYPE_COMPLEX)
       nregs = 2;
     else
       nregs = 0;
     if (is_subscript(node))
       {
	sptr = get_scalar_info_ptr(gen_SUBSCRIPT_get_name(node));
	if (sptr->generator != -1 || !sptr->is_generator)
	  put_label(node,0);
	else
	  put_label(node,nregs);
	if (get_label(node) > reg_info->expr_regs)
          reg_info->expr_regs = get_label(node);
	return(WALK_CONTINUE);
       }
     if (is_identifier(node))
       {
	if (!is_subscript(tree_out(node)))
	  put_label(node,0);
	return(WALK_CONTINUE);
       }
     if (is_unary_minus(node))
       {
        put_label(node,get_label(gen_UNARY_MINUS_get_rvalue(node)));
	if (get_label(node) > reg_info->expr_regs)
          reg_info->expr_regs = get_label(node);
	return(WALK_CONTINUE);
       }
     if (is_binary_exponent(node))
       {
        label1 = get_label(gen_BINARY_EXPONENT_get_rvalue1(node));
        label2 = get_label(gen_BINARY_EXPONENT_get_rvalue2(node));
       }
     else if (is_binary_times(node))
       {
        label1 = get_label(gen_BINARY_TIMES_get_rvalue1(node));
        label2 = get_label(gen_BINARY_TIMES_get_rvalue2(node));
       }
     else if (is_binary_divide(node))
       {
        label1 = get_label(gen_BINARY_DIVIDE_get_rvalue1(node));
        label2 = get_label(gen_BINARY_DIVIDE_get_rvalue2(node));
       }
     else if (is_binary_plus(node))
       {
        label1 = get_label(gen_BINARY_PLUS_get_rvalue1(node));
        label2 = get_label(gen_BINARY_PLUS_get_rvalue2(node));
       }
     else if (is_binary_minus(node))
       {
        label1 = get_label(gen_BINARY_MINUS_get_rvalue1(node));
        label2 = get_label(gen_BINARY_MINUS_get_rvalue2(node));
       }
     else
       return(WALK_CONTINUE);
     if (label1 == label2)
       put_label(node,label1 + nregs);
     else
       {
	if (label1 > label2)
	  put_label(node,label1);
	else
	  put_label(node,label2);
       }
     if (get_label(node) > reg_info->expr_regs)
       reg_info->expr_regs = get_label(node);
     return(WALK_CONTINUE);
  }


static int count_regs(AST_INDEX     stmt,
		      int           level,
		      reg_info_type *reg_info)

  {
   int stmt_regs;

     stmt_regs = reg_info->expr_regs;
     if (is_assignment(stmt))
       {
	walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
			(WK_EXPR_CLBACK)get_expr_regs,(Generic)reg_info);
       }    
     else if (is_guard(stmt))
       if (gen_GUARD_get_rvalue(stmt) != AST_NIL)
         walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,
			 (WK_EXPR_CLBACK)get_expr_regs,(Generic)reg_info);
       else;
     else if (is_arithmetic_if(stmt))
       walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,
		       (WK_EXPR_CLBACK)get_expr_regs,(Generic)reg_info);
     else if (is_logical_if(stmt))
       walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,
		       (WK_EXPR_CLBACK)get_expr_regs,(Generic)reg_info);
     else if (is_read_short(stmt))
       walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,
		       (WK_EXPR_CLBACK)get_expr_regs,(Generic)reg_info);
     else if (is_write(stmt))
       walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,
		       (WK_EXPR_CLBACK)get_expr_regs,(Generic)reg_info);
     if (stmt_regs > reg_info->expr_regs)
       reg_info->expr_regs = stmt_regs;
     return(WALK_CONTINUE);
  }


static int cleanup_gotos(AST_INDEX     stmt,
			 int           level,
			 SymDescriptor symtab)

  {
   if (is_goto(stmt))
     if (fst_GetField(symtab,gen_get_text(gen_GOTO_get_lbl_ref(stmt)),LBL_STMT)
	 == list_next(stmt) && gen_GOTO_get_lbl_def(stmt) == AST_NIL)
       (void)list_remove_node(stmt);
   return(WALK_CONTINUE);
  }

static int remove_dependences(AST_INDEX node,
			      PedInfo   ped)

  {
   AST_INDEX name;
   int       vector;
   EDGE_INDEX edge,
              next_edge;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	vector = get_info(ped,name,type_levelv);
	for (edge = dg_first_src_ref( PED_DG(ped),vector);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_ref( PED_DG(ped),edge);
	   dg_delete_free_edge( PED_DG(ped),edge);
	  }
	for (edge = dg_first_sink_ref( PED_DG(ped),vector);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_ref( PED_DG(ped),edge);
	   dg_delete_free_edge( PED_DG(ped),edge);
	  }
       }
     return(WALK_CONTINUE);
  }


static int compute_balance(AST_INDEX     node,
			   bal_info_type *bal_info)
	
  {
     if (is_subscript(node))
       {
	if (gen_get_converted_type(node) == TYPE_COMPLEX ||
	    gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION)
	  bal_info->mem += 
	     ((config_type *)PED_MH_CONFIG(bal_info->ped))->double_fetches;
	else 
	  bal_info->mem++;
       }
     else if (is_binary_op(node) || is_unary_minus(node))
        if (!is_binary_times(node) || !is_binary_plus(tree_out(node)) ||
	    !((config_type *)PED_MH_CONFIG(bal_info->ped))->mult_accum)
	  if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION ||
	      gen_get_converted_type(node) == TYPE_REAL)
            bal_info->flops++; 
	  else if (gen_get_converted_type(node) == TYPE_COMPLEX)
	    bal_info->flops += 2;
     return(WALK_CONTINUE);
  }


static void perform_scalar_replacement(do_info_type  *do_info,
				       AST_INDEX     root,
				       int           level)
				       
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {  
   AST_INDEX         loop_body; 
   prelim_info_type  prelim_info;
   flow_graph_type   flow_graph;
   check_info_type   check_info;
   gen_info_type     gen_info;
   name_info_type    name_info;
   UtilNode          *lnode;
   name_node_type    *name_node;
   reg_info_type     reg_info;
   Boolean           redo;
   FILE              *logfile;
   bal_info_type     bal_info;

     logfile  = ((config_type *)PED_MH_CONFIG(do_info->ped))->logfile;

     if (logfile != NULL)
       {

	/* ACCUMULATE BALANCE HERE */

	bal_info.mem = 0;
	bal_info.flops = 0;
	bal_info.ped = do_info->ped;
	walk_expression(root,(WK_EXPR_CLBACK)compute_balance,NOFUNC,
			(Generic)&bal_info);
	if (bal_info.flops > 0)
         {
	  fprintf(logfile,"Final Loop Balance = %.4f\n",
		  ((float) bal_info.mem)/((float) bal_info.flops));
          do_info->LoopStats->LoopBal += 
		     (float) bal_info.mem/(float) bal_info.flops;
         }
	else
	 {
	  fprintf(logfile,"Final Loop Balance = 0.0\n");
	  do_info->LoopStats->LoopBal += 0.0;
         }
       }
     prelim_info.array_refs = 0;
     prelim_info.scalar_regs = 0;
     prelim_info.def_num = 0;
     prelim_info.stmt_num = 0;
     prelim_info.contains_cf = false;
     prelim_info.contains_goto_or_label = false;
     prelim_info.jumps_ok = true;
     prelim_info.premature_exit = false;
     prelim_info.backjump = false;
     prelim_info.illegal_jump = false;
     prelim_info.ped = do_info->ped;
     prelim_info.level = level;
     prelim_info.ar = do_info->ar;
     loop_body = gen_DO_get_stmt_LIST(root);
     prelim_info.surrounding_do = do_info->do_num;
     fst_InitField(do_info->symtab,LBL_STMT,AST_NIL,0);
     fst_InitField(do_info->symtab,REFS,0,0);
     fst_InitField(do_info->symtab,NEW_LBL_INDEX,SYM_INVALID_INDEX,0);
     prelim_info.symtab = do_info->symtab;
     prelim_info.UniformRefs = 0;
     prelim_info.NonUniformRefs = 0;
     walk_statements(root,level,(WK_STMT_CLBACK)get_prelim_info,NOFUNC,
		     (Generic)&prelim_info);
     if (prelim_info.contains_cf && ReplaceLevel < 5)
       return;
     if (prelim_info.contains_goto_or_label)
       {
	walk_statements(loop_body,level,(WK_STMT_CLBACK)check_gotos,NOFUNC,
			(Generic)&prelim_info);
	walk_statements(loop_body,level,(WK_STMT_CLBACK)check_illegal_jumps,NOFUNC,
			(Generic)&prelim_info);
	if (!prelim_info.jumps_ok)
	  {
	   
	   /* INCREMENT BAD CONTROL FLOW HERE */
	   ++ do_info->LoopStats->Numbadflow;
	   if(prelim_info.premature_exit == true)
              ++ do_info->LoopStats->NumLoop_badexit;
           else if (prelim_info.backjump == true)
	      ++ do_info->LoopStats->NumLoop_backjump;
	   else if (prelim_info.illegal_jump)
	      ++ do_info->LoopStats->NumLoop_illjump;
	   else printf("What kind bad flow !!\n");

	   if (logfile != NULL)
	     fprintf(logfile,"bad control flow\n");
	   return;
	  }
       }


     prelim_info.array_table = (array_table_type *)
       do_info->ar->arena_alloc_mem_clear(LOOP_ARENA,prelim_info.array_refs*
					  sizeof(array_table_type));
     walk_statements(loop_body,level,(WK_STMT_CLBACK)sr_build_table,NOFUNC,
		     (Generic)&prelim_info);
     if (prelim_info.array_refs == 0)
       return;

       /* increment the number of loops replaced */ 
     ++ do_info->LoopStats->NumLoopReplaced;
     if (prelim_info.NonUniformRefs > 0)
       do_info->LoopStats->NonUniformLoopsReplaced++;
     do_info->LoopStats->UniformRefs += prelim_info.UniformRefs;
     do_info->LoopStats->NonUniformRefs += prelim_info.NonUniformRefs;

     check_info.size = prelim_info.array_refs;
     check_info.LC_kill = ut_create_set(do_info->ar,LOOP_ARENA,
					check_info.size);
     check_info.ped = do_info->ped;
     check_info.ar = do_info->ar;
     check_info.level = level;
     sr_check_inconsistent_edges(loop_body,&check_info);
     sr_build_flow_graph(&flow_graph,loop_body,prelim_info.symtab,do_info->ar,
			 do_info->LoopStats);
     sr_perform_avail_analysis(flow_graph,check_info);
     sr_perform_rgen_analysis(flow_graph,check_info);
     sr_pick_possible_generators(flow_graph,level,&prelim_info,do_info->ped);
     sr_perform_antic_analysis(flow_graph,prelim_info.array_refs,do_info->ped,
			       do_info->ar);
     gen_info.entry = flow_graph.entry;
     gen_info.level = level;
     gen_info.ped = do_info->ped;
     gen_info.array_table = prelim_info.array_table;
     sr_prune_graph(loop_body,level,&gen_info);
     if (ReplaceLevel > 4 && ReplaceLevel < 8)
       {

	/* if we have removed some partially available generators, we must redo
	   avail and antic analysis */
	sr_redo_gen_avail(flow_graph,prelim_info.array_refs,do_info->ped,do_info->ar);
	sr_perform_antic_analysis(flow_graph,prelim_info.array_refs,do_info->ped,
				  do_info->ar);
       }
     name_info.ped = do_info->ped;
     name_info.dg = dg_get_edge_structure( PED_DG(do_info->ped));
     name_info.glist = util_list_alloc((Generic)NULL,"generator-list");
     name_info.ar = do_info->ar;
     sr_generate_names(root,&name_info);
     if (!util_list_empty(name_info.glist))
       {
	reg_info.expr_regs = 0;
	reg_info.config = (config_type *)PED_MH_CONFIG(do_info->ped);
	walk_statements(loop_body,level,NOFUNC,(WK_STMT_CLBACK)count_regs,
			(Generic)&reg_info);
	if (((config_type *)PED_MH_CONFIG(do_info->ped))->chow_alloc && 
	    reg_info.expr_regs < 4)

	  /* reserve at least 4 register for a Chow-style register allocator
	     for expressions because of high interference */

	  prelim_info.scalar_regs += 4;
	else
	  prelim_info.scalar_regs += reg_info.expr_regs;
	sr_moderate_pressure(do_info->ped,name_info.glist,
			     ((config_type *)PED_MH_CONFIG(do_info->ped))
			     ->max_regs - prelim_info.scalar_regs,&redo,
			     prelim_info.array_table,logfile,do_info->ar,
			     do_info->LoopStats);
	if (!util_list_empty(name_info.glist))
	  {
	   if (redo)
	     {
	      
	      /* somebody's generator has changed, so this must be redone to
		 make the insert analysis correct */
	      
	      sr_redo_gen_avail(flow_graph,prelim_info.array_refs,
				do_info->ped,do_info->ar);
	      sr_perform_antic_analysis(flow_graph,prelim_info.array_refs,
					do_info->ped,do_info->ar);
	     }
	   sr_perform_insert_analysis(flow_graph,prelim_info.array_refs,
				      do_info->ar,do_info->ped);
	   sr_generate_code(root,do_info->ped,level,flow_graph,
			    prelim_info.array_table,prelim_info.symtab,
			    name_info.glist,logfile,do_info->ar);
	  }
	for (lnode = UTIL_HEAD(name_info.glist);
	     lnode != NULLNODE;
	     lnode = UTIL_NEXT(lnode))
	  {
	   name_node = (name_node_type *)UTIL_NODE_ATOM(lnode);
	   util_free_nodes(name_node->nlist);
	   util_list_free(name_node->nlist);
	  }
	util_free_nodes(name_info.glist);
       }
     else if (logfile != NULL)
       {
           /* INCREMENT FP COUNTER HERE */
         fprintf(logfile,"No FP Register Pressure\n");
         ++ do_info->LoopStats->NumZeroFPLoop;
	 if (prelim_info.NonUniformRefs > 0)
	   do_info->LoopStats->NonUniformLoopsZeroFP++;
       }

     util_list_free(name_info.glist);
     walk_expression(root,(WK_EXPR_CLBACK)remove_dependences,NOFUNC,
		     (Generic)do_info->ped);
     walk_statements(root,level,NOFUNC,(WK_STMT_CLBACK)cleanup_gotos,
		     (Generic)prelim_info.symtab);
     fst_KillField(do_info->symtab,LBL_STMT);
     fst_KillField(do_info->symtab,REFS);
     fst_KillField(do_info->symtab,NEW_LBL_INDEX);
  }

static int set_surrounding_do(AST_INDEX node,
			      arena_type *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX name;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	create_scalar_info_ptr(name,ar);
	get_scalar_info_ptr(name)->surrounding_do = -1;
       }
     return(WALK_CONTINUE);
  }

static int pre_scalar(AST_INDEX     stmt,
		      int           level,
		      do_info_type  *do_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   create_NULL_stmt_info_ptr(stmt);
   if (is_assignment(stmt))
     {
      walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),
		      (WK_EXPR_CLBACK)set_surrounding_do,NOFUNC,
		      (Generic)do_info->ar);
      walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),
		      (WK_EXPR_CLBACK)set_surrounding_do,
		      NOFUNC,(Generic)do_info->ar);
     }
   else if (is_guard(stmt))
     walk_expression(gen_GUARD_get_rvalue(stmt),
		     (WK_EXPR_CLBACK)set_surrounding_do,NOFUNC,
		     (Generic)do_info->ar);
   else if (is_write(stmt))
     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),
		     (WK_EXPR_CLBACK)set_surrounding_do,
		     NOFUNC,(Generic)do_info->ar);
   else if (is_read_short(stmt))
     walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),
		     (WK_EXPR_CLBACK)set_surrounding_do,NOFUNC,
		     (Generic)do_info->ar);
   else if (is_logical_if(stmt))
     walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),
		     (WK_EXPR_CLBACK)set_surrounding_do,NOFUNC,
		     (Generic)do_info->ar);
   else if (is_arithmetic_if(stmt))
     walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),
		     (WK_EXPR_CLBACK)set_surrounding_do,
		     NOFUNC,(Generic)do_info->ar);
   else if (is_do(stmt))
     {
      walk_expression(gen_DO_get_control(stmt),
		      (WK_EXPR_CLBACK)remove_dependences,NOFUNC,
		      (Generic)do_info->ped);
      do_info->inner_do = 0;
     }
   else if (is_call(stmt))
     walk_expression(gen_CALL_get_invocation(stmt),
		     (WK_EXPR_CLBACK)set_surrounding_do,NOFUNC,
		     (Generic)do_info->ar);
   else if (is_if(stmt) || is_continue(stmt) || is_goto(stmt) ||
	    is_computed_goto(stmt) || is_assigned_goto(stmt) || 
	    is_format(stmt) || is_stop(stmt));
   else if (executable_stmt(stmt))
     { 
      char errmsg[30];

      if (!is_return(stmt))
	{
	 sprintf(errmsg,"Statement not handled %d\n",NT(stmt));	 
	 message(errmsg);
	}
      do_info->abort = true;
     }
   return(WALK_CONTINUE);
  }

static int post_scalar(AST_INDEX     stmt,
		       int           level,
		       do_info_type  *do_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   if is_do(stmt) 
     {
      if (do_info->inner_do == 0)
        {
          if (!do_info->abort)
	    {
	     do_info->do_num++;
	     ++ do_info->LoopStats->NumInnermostLoop; 
	     perform_scalar_replacement(do_info,stmt,level);
	    }
	  else
	    do_info->abort = false;
	}
      do_info->inner_do = 1;
     }
   return(WALK_FROM_OLD_NEXT);
  }

void memory_scalar_replacement(PedInfo      ped,
			       AST_INDEX    root,
			       int          level,
			       SymDescriptor symtab,
			       arena_type    *ar,
			       LoopStatsType *LoopStats)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   do_info_type do_info;

     do_info.inner_do = 1;
     do_info.do_num = 0;
     do_info.ped = ped;
     do_info.abort = false;
     do_info.symtab = symtab;
     do_info.ar = ar;
     do_info.LoopStats = LoopStats;
     walk_statements(root,level,(WK_STMT_CLBACK)pre_scalar,
		     (WK_STMT_CLBACK)post_scalar,(Generic)&do_info);
  }
