/* $Id: prefetch.C,v 1.7 1995/08/18 10:20:54 trsuchyt Exp $ */

#include <mh.h>
#include <fort/gi.h>
#include <prefetch.h>
#include <analyze.h>
#include <shape.h>
#include <mem_util.h>
#include <mark.h>
#include <mh_ast.h>
#include <pt_util.h>
#include <fort/walk.h>
#include <dt.h>
#include <mh_config.h>
#include <misc/sllist.h>
#include <label.h>
#include <strings.h>

#include	<dg.h>

#include <UniformlyGeneratedSets.h>


static int remove_edges(AST_INDEX stmt,
			int       level,
			PedInfo   ped)

  {
   DG_Edge    *dg;
   int        vector;
   EDGE_INDEX edge,
              next_edge;
   int        i;

     dg = dg_get_edge_structure( PED_DG(ped));
     vector = get_info(ped,stmt,type_levelv);
     if (is_assignment(stmt))
       if (is_subscript(gen_ASSIGNMENT_get_lvalue(stmt)))
         get_subscript_ptr(gen_SUBSCRIPT_get_name(gen_ASSIGNMENT_get_lvalue(
			   stmt)))->store = true;
     for (i = 1;i <= level; i++)
       {
	for (edge = dg_first_src_stmt( PED_DG(ped),vector,i);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_stmt( PED_DG(ped),edge);
	   if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	       dg[edge].type == dg_call || dg[edge].type == dg_control)
	     dg_delete_free_edge( PED_DG(ped),edge);
	  }
	for (edge = dg_first_sink_stmt( PED_DG(ped),vector,i);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_sink_stmt( PED_DG(ped),edge);
	   if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	       dg[edge].type == dg_call || dg[edge].type == dg_control)
	     dg_delete_free_edge( PED_DG(ped),edge);
	  }
       }
     for (edge = dg_first_src_stmt( PED_DG(ped),vector,LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_src_stmt( PED_DG(ped),edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control)
	  dg_delete_free_edge( PED_DG(ped),edge);
       }
     for (edge = dg_first_sink_stmt( PED_DG(ped),vector,LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_sink_stmt( PED_DG(ped),edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control)
	   dg_delete_free_edge( PED_DG(ped),edge);
       }
     return(WALK_CONTINUE);
  }


static int CheckLocality(AST_INDEX          node,
			 locality_info_type *locality_info)

  {
   AST_INDEX name;
   subscript_info_type *sptr;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	sptr = get_subscript_ptr(name);
	sptr->Locality = ut_GetReferenceType(node,locality_info->loop_data,
					     locality_info->loop,
					     locality_info->ped,
					     locality_info->UGS);
	return(WALK_SKIP_CHILDREN);
       }
     return(WALK_CONTINUE);
  }


static int BuildPrefetchList(AST_INDEX          node,
			     locality_info_type *locality_info)

  {
   AST_INDEX name;
   subscript_info_type *sptr;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	sptr = get_subscript_ptr(name);
	if (sptr->Locality == NONE)
          if (((config_type *)PED_MH_CONFIG(locality_info->ped))->
	      write_allocate)
	    locality_info->WordPrefetches->append_entry(node); 
	  else if (NOT(sptr->store))
	    locality_info->WordPrefetches->append_entry(node); 
	  else if (sptr->uses_regs)
	    locality_info->WordPrefetches->append_entry(node); 
	  else;
	if (sptr->Locality == SELF_SPATIAL)
	  if (gen_get_real_type(node) == TYPE_REAL)
	    locality_info->SPLinePrefetches->append_entry(node); 
	  else
	    locality_info->DPLinePrefetches->append_entry(node); 
	return(WALK_SKIP_CHILDREN);
       }
     return(WALK_CONTINUE);
  }


static void CheckRefsForPrefetch(model_loop    *loop_data,
				 int           loop,
				 PrefetchList  *SPLinePrefetches,
				 PrefetchList  *DPLinePrefetches,
				 PrefetchList  *WordPrefetches,
				 PedInfo       ped,
				 char          **IVar)

  {
   locality_info_type locality_info;

     locality_info.ped = ped;
     locality_info.loop = loop;
     locality_info.loop_data = loop_data;
     locality_info.SPLinePrefetches = SPLinePrefetches;
     locality_info.DPLinePrefetches = DPLinePrefetches;
     locality_info.WordPrefetches = WordPrefetches;
     if (mc_extended_cache)
       locality_info.UGS = new UniformlyGeneratedSets(loop_data[loop].node,
						      loop_data[loop].level,
						      IVar);
     else
       locality_info.UGS = NULL;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
		     (WK_EXPR_CLBACK)CheckLocality,NOFUNC,(Generic)&locality_info);
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
		     (WK_EXPR_CLBACK)BuildPrefetchList,NOFUNC,
		     (Generic)&locality_info);
     delete locality_info.UGS;
  }
   

static int LoadCycles(AST_INDEX Node,
		      PedInfo   ped)

  {
   int LoadPenalty;

    LoadPenalty = ((config_type *)PED_MH_CONFIG(ped))->hit_cycles;
    if (gen_get_converted_type(Node) == TYPE_REAL)
      return(LoadPenalty);
    else if (gen_get_converted_type(Node) == TYPE_DOUBLE_PRECISION ||
            gen_get_converted_type(Node) == TYPE_COMPLEX)
      return(((config_type *)PED_MH_CONFIG(ped))->double_fetches*LoadPenalty);
    else	
      return(0);
  }


static int OperationCycles(AST_INDEX Node,
			   PedInfo   ped)

  {
   int ops;
     
     if (!is_binary_times(Node) || 
	 (!is_binary_plus(tree_out(Node)) && 
	  !is_binary_minus(tree_out(Node))) ||
	 !((config_type *)PED_MH_CONFIG(ped))->mult_accum)
       if (gen_get_converted_type(Node) == TYPE_DOUBLE_PRECISION ||
	   gen_get_converted_type(Node) == TYPE_COMPLEX ||
	   gen_get_converted_type(Node) == TYPE_REAL)
	 {
	  if (gen_get_converted_type(Node) == TYPE_COMPLEX)
	    ops = 2;
	  else
	    ops = 1;
	  if (is_binary_times(Node))
	    return(((config_type *)PED_MH_CONFIG(ped))->mul_cycles * ops);
	  else if (is_binary_plus(Node) || is_binary_minus(Node))
	    return(((config_type *)PED_MH_CONFIG(ped))->add_cycles * ops);
	  else if (is_binary_divide(Node))
	    return(((config_type *)PED_MH_CONFIG(ped))->div_cycles * ops);
	  else
	    return(ops); 
	 }
     return(0);
  }


static int CountCycles(AST_INDEX     Node,
		       CycleInfoType *CycleInfo)

  {
   subscript_info_type *sptr;

     if (is_subscript(Node))
       {
	sptr = get_subscript_ptr(gen_SUBSCRIPT_get_name(Node));
	if (sptr->Locality == NONE || sptr->Locality == SELF_SPATIAL ||
	    sptr->Locality == GROUP_SPATIAL)
	  CycleInfo->MemCycles += LoadCycles(Node,CycleInfo->ped);
       }
     else if (is_binary_op(Node))
	CycleInfo->FlopCycles += OperationCycles(Node,CycleInfo->ped);
     return(WALK_CONTINUE);
  }


static int CyclesPerIteration(AST_INDEX Node,
			      PedInfo   ped)

  {
   CycleInfoType CycleInfo;
  
     CycleInfo.MemCycles = 0;
     CycleInfo.FlopCycles = 0;
     CycleInfo.ped = ped;
     walk_expression(gen_DO_get_stmt_LIST(Node),(WK_EXPR_CLBACK)CountCycles,NOFUNC,
		     (Generic)&CycleInfo);
     if (CycleInfo.MemCycles >= CycleInfo.FlopCycles)
       return(CycleInfo.MemCycles);
     else
       return(CycleInfo.FlopCycles);
  }

static void ModeratePrefetchRequirements(model_loop   *loop_data,
					 int          loop,
					 PrefetchList *SPLinePrefetches,
					 PrefetchList *DPLinePrefetches,
					 PrefetchList *WordPrefetches,
					 PedInfo      ped)

  {
   float PrefetchBandwidth;
   float BandwidthNeeded;
   float ScheduleBandwidth;
   float SPLineValue,DPLineValue;
   float Cycles;
   PrefetchListIterator SPLineIterator(SPLinePrefetches),
                        DPLineIterator(DPLinePrefetches),
                        WordIterator(WordPrefetches);
   PrefetchListEntry    *temp;

     if (((config_type *)PED_MH_CONFIG(ped))->prefetch_latency == 0)
       PrefetchBandwidth = 0.0;
     else
       PrefetchBandwidth = 
        ((float) ((config_type *)PED_MH_CONFIG(ped))->prefetch_buffer) /
        ((float) ((config_type *)PED_MH_CONFIG(ped))->prefetch_latency);
     BandwidthNeeded = WordPrefetches->Count();
     SPLineValue = ((float)SPLinePrefetches->Count()) *
                           (4.0/(float)((config_type *)PED_MH_CONFIG(ped))->line);
     DPLineValue = ((float)DPLinePrefetches->Count()) *
                           (8.0/(float)((config_type *)PED_MH_CONFIG(ped))->line);
     BandwidthNeeded += SPLineValue + DPLineValue;
     Cycles = CyclesPerIteration(loop_data[loop].node,ped);
     BandwidthNeeded /= Cycles;
     if (BandwidthNeeded > PrefetchBandwidth)
       {
	ScheduleBandwidth = 0.0;
	while (WordIterator.current() != NULL)
	  if (ScheduleBandwidth <= (PrefetchBandwidth - 1.0/Cycles))
	    {
	     ScheduleBandwidth += (1.0/Cycles);
	     (void)WordIterator++;
	    }
	  else
	    {
	     temp = WordIterator.current();
	     (void)WordIterator++;
	     WordPrefetches->Delete(temp);
	    }
	while (DPLineIterator.current() != NULL)
	  if (ScheduleBandwidth <= (PrefetchBandwidth - DPLineValue/Cycles))
	    {
	     ScheduleBandwidth += (DPLineValue/Cycles);
	     (void)DPLineIterator++;
	    }
	  else
	    {
	     temp = DPLineIterator.current();
	     (void)DPLineIterator++;
	     DPLinePrefetches->Delete(temp);
	    }
	while (SPLineIterator.current() != NULL)
	  if (ScheduleBandwidth <= (PrefetchBandwidth - SPLineValue/Cycles))
	    {
	     ScheduleBandwidth += (SPLineValue/Cycles);
	     (void)SPLineIterator++;
	    }
	  else
	    {
	     temp = SPLineIterator.current();
	     (void)SPLineIterator++;
	     SPLinePrefetches->Delete(temp);
	    }
       }
  }


static AST_INDEX MakePrefetchStmt(AST_INDEX    Node,
				  AST_INDEX    Var,
				  int          distance)

  {
   AST_INDEX PrefetchNode;
   char      Text[80],
             Instruction[100];

           /* generate a function call so that pt_var_add will update references
	      to the induction variable.  Later the call is changed to a 
              directive */

     PrefetchNode = tree_copy_with_type(Node);
     pt_var_add(PrefetchNode,gen_get_text(Var),distance);
     return(pt_gen_call("$$Prefetch",list_create(pt_simplify_expr(PrefetchNode))));
  }


static void InsertPrefetchesBeforeStmt(AST_INDEX    Stmt,
				       PrefetchList *Prefetches,
				       AST_INDEX    Var,
				       int          distance)

  {
   PrefetchListIterator Iterator(Prefetches);
   PrefetchListEntry    *Node;

     for (Node = Iterator.current();
	  Node != NULL;
	  Node = Iterator.current())
       {
	if (Stmt != AST_NIL)
          list_insert_before(Stmt,MakePrefetchStmt(Node->GetValue(),Var,
						   distance));
	else
         list_insert_before(ut_get_stmt(Node->GetValue()),
			    MakePrefetchStmt(Node->GetValue(),Var,distance));
	Iterator++;
       }
  }


static int PipelineIterations(int minimum,
			      int iteration)

  {
   if (minimum >= iteration)
     return minimum;
   else
     return minimum * ceil_ab(iteration,minimum);
  }

static void PrimePrefetchPipe(AST_INDEX    Node,
			      int          WordDistance,
			      int          LineDistance,
			      PrefetchList *WordPrefetches,
			      PrefetchList *SPLinePrefetches,
			      PrefetchList *DPLinePrefetches)

  {
   AST_INDEX NewLoop,
             NewCtrl,
             Stmt,
             NextStmt,
             Step,
             ArgList,
             Min,
             Var;
   int       StepVal;

     NewLoop = tree_copy_with_type(Node);
     Stmt = list_first(gen_DO_get_stmt_LIST(NewLoop));
     NewCtrl = gen_DO_get_control(NewLoop);
     Var = gen_INDUCTIVE_get_name(NewCtrl);
     ArgList = list_create(pt_simplify_expr(pt_gen_add(
			         tree_copy_with_type(gen_INDUCTIVE_get_rvalue1(NewCtrl)),
				 pt_gen_int(WordDistance-1))));
     ArgList = list_insert_first(ArgList,
				tree_copy_with_type(gen_INDUCTIVE_get_rvalue2(NewCtrl)));
     Min = gen_INVOCATION(pt_gen_ident("min"),ArgList);
     pt_tree_replace(gen_INDUCTIVE_get_rvalue2(NewCtrl),Min);
     InsertPrefetchesBeforeStmt(Stmt,WordPrefetches,Var,0);
     while (Stmt != AST_NIL)
       {
	NextStmt = list_next(Stmt);
	tree_free(list_remove_node(Stmt));
	Stmt = NextStmt;
       }
     list_insert_before(Node,NewLoop);

     
     NewLoop = tree_copy_with_type(Node);
     Stmt = list_first(gen_DO_get_stmt_LIST(NewLoop));
     NewCtrl = gen_DO_get_control(NewLoop);
     Var = gen_INDUCTIVE_get_name(NewCtrl);
     ArgList = list_create(pt_simplify_expr(pt_gen_add(
			         tree_copy_with_type(gen_INDUCTIVE_get_rvalue1(NewCtrl)),
				 pt_gen_int(LineDistance*2 - 1))));
     ArgList = list_insert_first(ArgList,
				tree_copy_with_type(gen_INDUCTIVE_get_rvalue2(NewCtrl)));
     Min = gen_INVOCATION(pt_gen_ident("min"),ArgList);
     pt_tree_replace(gen_INDUCTIVE_get_rvalue2(NewCtrl),Min);
     Step = gen_INDUCTIVE_get_rvalue3(NewCtrl);
     if (Step == AST_NIL)
       gen_INDUCTIVE_put_rvalue3(NewCtrl,pt_gen_int(LineDistance));
     else 
       {
	(void)pt_eval(Step,&StepVal);
	pt_tree_replace(Step,pt_gen_mul(pt_gen_int(LineDistance),pt_gen_int(StepVal)));
       }
     InsertPrefetchesBeforeStmt(Stmt,SPLinePrefetches,Var,0);
     InsertPrefetchesBeforeStmt(Stmt,DPLinePrefetches,Var,0);
     while (Stmt != AST_NIL)
       {
	NextStmt = list_next(Stmt);
	tree_free(list_remove_node(Stmt));
	Stmt = NextStmt;
       }
     list_insert_before(Node,NewLoop);
  }

static void PeelLoop(AST_INDEX     Node,
		     int           Iterations,
		     int           WordDistance,
		     int           LineDistance,
		     PrefetchList *WordPrefetches)
  {
   AST_INDEX NewLoop,
             NewCtrl,
             Ctrl,
             UpBnd,
             ArgList,
             Max,
             Var,
             Iter;
   int       i;

     NewLoop = tree_copy_with_type(Node);
     NewCtrl = gen_DO_get_control(NewLoop);
     Ctrl = gen_DO_get_control(Node);
     ArgList = list_create(pt_simplify_expr(pt_gen_sub(
			         tree_copy_with_type(gen_INDUCTIVE_get_rvalue2(NewCtrl)),
				 pt_gen_int(WordDistance-1))));
     ArgList = list_insert_first(ArgList,
				tree_copy_with_type(gen_INDUCTIVE_get_rvalue1(NewCtrl)));
     Max = gen_INVOCATION(pt_gen_ident("max"),ArgList);
     pt_tree_replace(gen_INDUCTIVE_get_rvalue1(NewCtrl),Max);
     list_insert_after(Node,NewLoop);

     Var = gen_INDUCTIVE_get_name(Ctrl);
     NewLoop = tree_copy_with_type(Node);
     InsertPrefetchesBeforeStmt(list_first(gen_DO_get_stmt_LIST(NewLoop)),
				WordPrefetches,Var,WordDistance);
     NewCtrl = gen_DO_get_control(NewLoop);
     ArgList = list_create(pt_simplify_expr(pt_gen_sub(
				 tree_copy_with_type(gen_INDUCTIVE_get_rvalue2(NewCtrl)),
				 pt_gen_int(LineDistance*2-1))));
     ArgList = list_insert_first(ArgList,
				tree_copy_with_type(gen_INDUCTIVE_get_rvalue1(NewCtrl)));
     Max = gen_INVOCATION(pt_gen_ident("max"),ArgList);
     pt_tree_replace(gen_INDUCTIVE_get_rvalue1(NewCtrl),Max);
     UpBnd = pt_simplify_expr(pt_gen_sub(
				 tree_copy_with_type(gen_INDUCTIVE_get_rvalue2(NewCtrl)),
				 pt_gen_int(WordDistance)));
     pt_tree_replace(gen_INDUCTIVE_get_rvalue2(NewCtrl),UpBnd);
     list_insert_after(Node,NewLoop);
     pt_tree_replace(gen_INDUCTIVE_get_rvalue2(Ctrl),
		     pt_simplify_expr(pt_gen_sub(tree_copy_with_type(
                                                 gen_INDUCTIVE_get_rvalue2(Ctrl)),
						 pt_gen_int(LineDistance*2))));
  }

static void CreatePreLoop(model_loop    *loop_data,
			  int           loop,
			  int           UnrollVal,
			  SymDescriptor Symtab)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
  

  {
   AST_INDEX control,new_loop,next,
             lwb,upb,step;
   Boolean   need_pre_loop = false;
   int       lwb_v,upb_v,step_v;

     control = gen_DO_get_control(loop_data[loop].node);
     lwb = gen_INDUCTIVE_get_rvalue1(control);
     if (pt_eval(lwb,&lwb_v))
       need_pre_loop = true;
     else
       {
	upb = gen_INDUCTIVE_get_rvalue2(control);
	if (pt_eval(upb,&upb_v))
	  need_pre_loop = true;
	else
	  {
	   step = gen_INDUCTIVE_get_rvalue3(control);
	   if (step == AST_NIL)
	     step_v = 1;
	   else if (pt_eval(step,&step_v))
	     need_pre_loop = true;
	   if (!need_pre_loop)
	     if (mod((upb_v - lwb_v + 1)/step_v, loop_data[loop].val + 1)
	           != 0)
	       need_pre_loop = true;
	  }
       }
     if (need_pre_loop)
       {
	new_loop = tree_copy_with_type(loop_data[loop].node);
	ut_update_labels(new_loop,Symtab);
	ut_update_bounds(loop_data[loop].node,new_loop,UnrollVal);
	list_insert_before(loop_data[loop].node,new_loop);
       }
     else 
       ut_update_bounds(loop_data[loop].node,AST_NIL,UnrollVal);
  }
  

static void ReplicateBody(AST_INDEX DoNode,
			  int       UnrollVal,
			  AST_INDEX Ivar,
			  SymDescriptor Symtab,
			  arena_type    *ar)
  
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX     *NewCode,
                 Step,
                 StmtList;
   int           i,
                 StepVal;
   
     StmtList = gen_DO_get_stmt_LIST(DoNode);
     Step = gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(DoNode));
     if (Step == AST_NIL)
       StepVal = 1;
     else 
       {
	(void)pt_eval(Step,&StepVal);
	StepVal /= (UnrollVal+1);
       }
     NewCode = (AST_INDEX *)ar->arena_alloc_mem(LOOP_ARENA,
						UnrollVal * sizeof(AST_INDEX));
     for (i = 0; i < UnrollVal-1; i++)
       {
	NewCode[i] = tree_copy_with_type(StmtList);
	pt_var_add(NewCode[i],gen_get_text(Ivar),StepVal*(i+1));
	ut_update_labels(NewCode[i],Symtab);
       }
     NewCode[UnrollVal-1] = tree_copy_with_type(StmtList);
     pt_var_add(NewCode[UnrollVal-1],gen_get_text(Ivar),StepVal*UnrollVal);
     ut_update_labels(StmtList,Symtab);
     for (i = 0; i < UnrollVal; i++)
       StmtList = list_append(StmtList,NewCode[i]);
  }
	

static void UnrollLoop(model_loop   *loop_data,
		       int           loop,
		       PrefetchList *SPLinePrefetches,
		       PrefetchList *DPLinePrefetches,
		       int           LineDistance,
		       AST_INDEX     Var,
		       SymDescriptor Symtab,
		       arena_type    *ar)
  {
   AST_INDEX Stmt;
   int       i;

     CreatePreLoop(loop_data,loop,LineDistance-1,Symtab);
     ReplicateBody(loop_data[loop].node,LineDistance-1,Var,Symtab,ar);
     InsertPrefetchesBeforeStmt(list_first(gen_DO_get_stmt_LIST(loop_data[loop].node)),
				DPLinePrefetches,Var,LineDistance*2);
     InsertPrefetchesBeforeStmt(list_first(gen_DO_get_stmt_LIST(loop_data[loop].node)),
				SPLinePrefetches,Var,LineDistance*2);
  }

static void StripLoop(model_loop   *loop_data,
		      int           loop,
		      PrefetchList *SPLinePrefetches,
		      PrefetchList *DPLinePrefetches,
		      int           LineDistance,
		      AST_INDEX     Var,
		      SymDescriptor Symtab,
		      arena_type    *ar)
  {
   AST_INDEX NewLoop,
             Ctrl,
             Ivar,
             StmtList,
             ArgList,
             Min,
             Step;
   int       StepVal;
   char      NewIvar[80];

     StmtList = gen_DO_get_stmt_LIST(loop_data[loop].node);
     gen_DO_put_stmt_LIST(loop_data[loop].node,AST_NIL);
     Ctrl = gen_DO_get_control(loop_data[loop].node);
     Ivar = gen_INDUCTIVE_get_name(Ctrl);
     sprintf(NewIvar,"%s$%d",gen_get_text(Ivar),loop_data[loop].level);
     Step = gen_INDUCTIVE_get_rvalue3(Ctrl);
     if (Step == AST_NIL)
       StepVal = 1;
     else 
       (void)pt_eval(Step,&StepVal);
     ArgList = list_create(pt_gen_add(tree_copy_with_type(Ivar),
				      pt_gen_int((LineDistance-1)*StepVal)));
     ArgList = list_insert_last(ArgList,
				tree_copy_with_type(gen_INDUCTIVE_get_rvalue2(Ctrl)));
     Min = gen_INVOCATION(pt_gen_ident("min"),ArgList);
     NewLoop = gen_DO(AST_NIL,AST_NIL,AST_NIL,
		      gen_INDUCTIVE(pt_gen_ident(NewIvar),tree_copy_with_type(Ivar),
				    Min,tree_copy_with_type(Step)),
		      StmtList);
   
     if (Step == AST_NIL)
       gen_INDUCTIVE_put_rvalue3(Ctrl,pt_gen_int(LineDistance*StepVal));
     else
       pt_tree_replace(gen_INDUCTIVE_get_rvalue3(Ctrl),
		       pt_gen_int(LineDistance*StepVal));
     gen_DO_put_stmt_LIST(loop_data[loop].node,list_create(NewLoop));
     InsertPrefetchesBeforeStmt(NewLoop,DPLinePrefetches,Var,LineDistance*2);
     InsertPrefetchesBeforeStmt(NewLoop,SPLinePrefetches,Var,LineDistance*2);
     pt_var_replace(StmtList,gen_get_text(Ivar),pt_gen_ident(NewIvar));
  }

static void SchedulePrefetches(model_loop *loop_data,
			       int        loop,
			       PrefetchList *SPLinePrefetches,
			       PrefetchList *DPLinePrefetches,
			       PrefetchList *WordPrefetches,
			       PedInfo      ped,
			       SymDescriptor symtab,
			       arena_type    *ar)
  {
   AST_INDEX var;
   int       SPLinePrefetchDistance,
             DPLinePrefetchDistance,
             LineDistance,
             WordPrefetchDistance,
             MaxDistance,
             IterationDist,
             Cycles;

     Cycles = CyclesPerIteration(loop_data[loop].node,ped);
     IterationDist = ceil_ab(((config_type *)PED_MH_CONFIG(ped))->
			     prefetch_latency,Cycles);
     if (NOT(WordPrefetches->NullList()))
       WordPrefetchDistance = PipelineIterations(1,IterationDist);
     else
       WordPrefetchDistance = 0;
     if (NOT(SPLinePrefetches->NullList()))
       SPLinePrefetchDistance =
	    PipelineIterations(((config_type *)PED_MH_CONFIG(ped))->line >> 2,
			       IterationDist);
     else
       SPLinePrefetchDistance = 0;
     if (NOT(DPLinePrefetches->NullList()))
       DPLinePrefetchDistance =
	    PipelineIterations(((config_type *)PED_MH_CONFIG(ped))->line >> 3,
			       IterationDist);
     else
       DPLinePrefetchDistance = 0;
     LineDistance = DPLinePrefetchDistance == 0 ? SPLinePrefetchDistance : 
                                                  DPLinePrefetchDistance;
     MaxDistance = LineDistance  > WordPrefetchDistance ? LineDistance :
                                                          WordPrefetchDistance; 
     if (MaxDistance != 0)
       {
	PeelLoop(loop_data[loop].node,MaxDistance,WordPrefetchDistance,
		 DPLinePrefetchDistance > 0 ? DPLinePrefetchDistance : 
		 SPLinePrefetchDistance,WordPrefetches);
	PrimePrefetchPipe(loop_data[loop].node,WordPrefetchDistance,LineDistance,
			  WordPrefetches,SPLinePrefetches,DPLinePrefetches);
	var = gen_INDUCTIVE_get_name(gen_DO_get_control(loop_data[loop].node));
	InsertPrefetchesBeforeStmt(AST_NIL,WordPrefetches,var,WordPrefetchDistance);
	if (NOT(SPLinePrefetches->NullList()) || NOT(DPLinePrefetches->NullList()))
	  if (LineDistance <= 8)
	    UnrollLoop(loop_data,loop,SPLinePrefetches,DPLinePrefetches,LineDistance,var,
		       symtab,ar);
	  else
	    StripLoop(loop_data,loop,SPLinePrefetches,DPLinePrefetches,LineDistance,var,
		      symtab,ar);
       }
  }


static int ConvertPrefetchCallsToDirectives(AST_INDEX stmt,
					    int       level,
					    int       dummy)

  {
   char Text[80],
        Instruction[100];
   AST_INDEX Inv;

     if (is_call(stmt))
       {
	Inv = gen_CALL_get_invocation(stmt);
	if (strcmp("$$Prefetch",gen_get_text(gen_INVOCATION_get_name(Inv))) == 0)
	  {
	   ut_GetSubscriptText(list_first(gen_INVOCATION_get_actual_arg_LIST(Inv)),
			       Text);
	   sprintf(Instruction,"prefetch (%s)",Text);
	   pt_tree_replace(stmt,pt_gen_comment(Instruction));
	   return(WALK_FROM_OLD_NEXT);
	  }
       }
     return(WALK_CONTINUE);
  }


static void walk_loops(model_loop    *loop_data,
		       int           loop,
		       PedInfo       ped,
		       SymDescriptor symtab,
		       arena_type    *ar,
		       char          **IVar)
		       
  {
   int next;
   PrefetchList SPLinePrefetches;
   PrefetchList DPLinePrefetches;
   PrefetchList WordPrefetches;

     IVar[loop_data[loop].level-1] = gen_get_text(gen_DO_get_control(
                                     gen_INDUCTIVE_get_name(loop_data[loop].node)));

     if (loop_data[loop].inner_loop == -1)
       {
	CheckRefsForPrefetch(loop_data,loop,&SPLinePrefetches,&DPLinePrefetches,
			     &WordPrefetches,ped,IVar);
	if (!((config_type *)PED_MH_CONFIG(ped))->aggressive)
	  ModeratePrefetchRequirements(loop_data,loop,&SPLinePrefetches,
				       &DPLinePrefetches,&WordPrefetches,ped);
	SchedulePrefetches(loop_data,loop,&SPLinePrefetches,&DPLinePrefetches,
			   &WordPrefetches,ped,symtab,ar);
	walk_statements(tree_out(loop_data[loop].node),loop_data[loop].level,NOFUNC,
			(WK_STMT_CLBACK)ConvertPrefetchCallsToDirectives,
			(Generic)NULL);
       }
     else
       {
	for (next = loop_data[loop].inner_loop;
	     next != -1;
	     next = loop_data[next].next_loop)
	  walk_loops(loop_data,next,ped,symtab,ar,IVar);
       }
  }


void memory_software_prefetch(PedInfo       ped,
			      AST_INDEX     root,
			      int           level,
			      SymDescriptor symtab,
			      arena_type    *ar)

  {
   pre_info_type pre_info;
   model_loop    *loop_data;
   UtilList      *loop_list;
   char          **IVar;

     pre_info.stmt_num = 0;
     pre_info.loop_num = 0;
     pre_info.surrounding_do = -1;
     pre_info.abort = false;
     pre_info.ped = ped;
     pre_info.symtab = symtab;
     pre_info.ar = ar;
     walk_statements(root,level,(WK_STMT_CLBACK)ut_mark_do_pre,
		     (WK_STMT_CLBACK)ut_mark_do_post,(Generic)&pre_info);
     if (pre_info.abort)
       return;
     walk_statements(root,level,(WK_STMT_CLBACK)remove_edges,NOFUNC,(Generic)ped);
     loop_data = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
					 pre_info.loop_num*sizeof(model_loop));
     ut_analyze_loop(root,loop_data,level,ped,symtab);
     ut_check_shape(loop_data,0);
     IVar = new char*[pre_info.loop_num];
     walk_loops(loop_data,0,ped,symtab,ar,IVar);
     delete IVar;
  }
