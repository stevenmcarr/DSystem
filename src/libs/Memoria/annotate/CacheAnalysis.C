/* $Id: CacheAnalysis.C,v 1.24 1999/06/23 13:39:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

#include <iostream.h>
#include <assert.h>
#include <libs/support/misc/general.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/Memoria/include/mh_config.h>
#include <libs/Memoria/include/label.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/Memoria/include/bound.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/Memoria/include/header.h>
#include <libs/Memoria/include/mark.h>
#include <libs/Memoria/include/analyze.h>
#include <libs/Memoria/include/mem_util.h>
#include <libs/Memoria/annotate/CacheAnalysis.h>
#include <libs/Memoria/annotate/DirectivesInclude.h>
#include <libs/Memoria/include/la.h>

extern int aiCache;
extern int aiOptimizeAddressCode;

static int RefCount = 0;

static int remove_edges(AST_INDEX      stmt,
			int            level,
			PedInfo        ped)

  {
   DG_Edge    *dg;
   int        vector,lvl;
   EDGE_INDEX edge,
              next_edge;
   stmt_info_type *sptr;

     if (is_assignment(stmt))
       if (is_subscript(gen_ASSIGNMENT_get_lvalue(stmt)))
         get_subscript_ptr(gen_SUBSCRIPT_get_name(gen_ASSIGNMENT_get_lvalue(
                           stmt)))->store = true;
     dg = dg_get_edge_structure( PED_DG(ped));
     vector = get_info(ped,stmt,type_levelv);
     for (lvl = 1; lvl < level;lvl++)
       {
	for (edge = dg_first_src_stmt( PED_DG(ped),vector,lvl);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_stmt( PED_DG(ped),edge);
	   if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	       dg[edge].type == dg_call || dg[edge].type == dg_control)
	     dg_delete_free_edge( PED_DG(ped),edge);
	   else 
	     if ((sptr=get_stmt_info_ptr(ut_get_stmt(dg[edge].sink))) == NULL)
	       dg_delete_free_edge( PED_DG(ped),edge);
	  }
	for (edge = dg_first_sink_stmt( PED_DG(ped),vector,lvl);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_sink_stmt( PED_DG(ped),edge);
	   if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	       dg[edge].type == dg_call || dg[edge].type == dg_control)
	     dg_delete_free_edge( PED_DG(ped),edge);
	   else 
	     if ((sptr = get_stmt_info_ptr(ut_get_stmt(dg[edge].src))) == NULL)
	       dg_delete_free_edge( PED_DG(ped),edge);
	  }
       }
     for (edge = dg_first_src_stmt( PED_DG(ped),vector,
				   LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_src_stmt( PED_DG(ped),edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control)
	  dg_delete_free_edge( PED_DG(ped),edge);
	else 
	  if ((sptr = get_stmt_info_ptr(ut_get_stmt(dg[edge].sink))) == NULL)
	    dg_delete_free_edge( PED_DG(ped),edge);
       }
     for (edge = dg_first_sink_stmt( PED_DG(ped),vector,
				    LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_sink_stmt( PED_DG(ped),edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control)
	  dg_delete_free_edge( PED_DG(ped),edge);
	else 
	  if ((sptr = get_stmt_info_ptr(ut_get_stmt(dg[edge].src))) == NULL)
	    dg_delete_free_edge( PED_DG(ped),edge);
       }
     return(WALK_CONTINUE);
  }

static Boolean IsValidDependence(AST_INDEX PrefetchNode,
				 AST_INDEX RefNode,
				 char      *InnerIvar = NULL)

{
  AST_INDEX SubList1,SubList2;
  AST_INDEX Sub1, Sub2;

  if (strcmp(gen_get_text(gen_SUBSCRIPT_get_name(PrefetchNode)),
	     gen_get_text(gen_SUBSCRIPT_get_name(RefNode))) == 0)
    {
      SubList1 = gen_SUBSCRIPT_get_rvalue_LIST(PrefetchNode);
      SubList2 = gen_SUBSCRIPT_get_rvalue_LIST(RefNode);
      for (Sub1 = list_first(SubList1), Sub2 = list_first(SubList2);
	   Sub1 != AST_NIL && Sub2 != AST_NIL;
	   Sub1 = list_next(Sub1), Sub2 = list_next(Sub2))
	if (InnerIvar != NULL)
	  if (NOT(pt_find_var(Sub1,InnerIvar)))
	    if (NOT(pt_find_var(Sub2,InnerIvar)))
	      if (NOT(pt_expr_equal(Sub1,Sub2)))
		return false;
	      else;
	    else
	      return false;
	  else 
	    if (pt_find_var(Sub2,InnerIvar))
	      return false;
	    else;
	else
	  if (NOT(pt_expr_equal(Sub1,Sub2)))
	    return false;
      if (Sub1 != AST_NIL || Sub2 != AST_NIL)
	return false;
      else
	return true;
    }
  else
    return false;
}

static void AddDependenceToDirective(Directive *Dir,
				     AST_INDEX Reference)
{
  DepStruct *Dep;
  AST_INDEX DepNode;

  Dep = new DepStruct;
  Dep->Reference = Reference;
  Dep->ReferenceNumber = -1;
  Dep->DType = 'p';
  Dep->Distance = 0;
  util_append(Dir->DependenceList,util_node_alloc((Generic)Dep,NULL));
}

static int BuildDependenceList(AST_INDEX node,CacheInfoType *CacheInfo)

  {
   DG_Edge    *dg;
   int        vector;
   EDGE_INDEX edge;
   AST_INDEX  name;
   DepStruct  *Dep;
   int        InnerLevel;
   DepInfoType *Dptr;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	InnerLevel = CacheInfo->loop_data[get_subscript_ptr(name)->surrounding_do].level;
	dg = dg_get_edge_structure( PED_DG(CacheInfo->ped));
	vector = get_info(CacheInfo->ped,name,type_levelv);
	for (edge = dg_first_src_ref(PED_DG(CacheInfo->ped),vector);
	     edge != END_OF_LIST;
	     edge = dg_next_src_ref(PED_DG(CacheInfo->ped),edge))
	  {
	   if ((dg[edge].type == dg_true || dg[edge].type == dg_anti ||
		dg[edge].type == dg_output) && 
	       (dg[edge].level == LOOP_INDEPENDENT || 
		dg[edge].level == InnerLevel))
	     if ((Dptr = DepInfoPtr(tree_out(dg[edge].sink))) != NULL)
	       {
		 Dep = new DepStruct;
		 Dep->ReferenceNumber = Dptr->ReferenceNumber;
		 if (dg[edge].type == dg_true) 
		   Dep->DType = 't';
		 else if (dg[edge].type == dg_anti) 
		   Dep->DType = 'a';
		 else
		   Dep->DType = 'o';
		 if (dg[edge].level == LOOP_INDEPENDENT)
		   Dep->Distance = 0;
		 else
		   {
		     Dep->Distance = gen_get_dt_DIS(&dg[edge],dg[edge].level);
		     if (Dep->Distance < DDATA_BASE)
		       Dep->Distance = 1;
		   }
		 util_append(DepInfoPtr(node)->DependenceList,
			     util_node_alloc((Generic)Dep,NULL));
	       }
	  }
       }
     return(WALK_CONTINUE);
  }

static void CreateDepInfoSubscript(AST_INDEX Id,
				   ReferenceAccessType aType,
				   va_list args)
{
  AST_INDEX node;
  SymDescriptor Symtab;

  Symtab = va_arg(args,SymDescriptor);
  if (fst_GetField(Symtab,gen_get_text(Id),SYMTAB_NUM_DIMS) > 0 && 
      (aType == at_ref || aType == at_mod))
    {
      node = tree_out(Id);
      CreateDepInfoPtr(node);
      DepInfoPtr(node)->DependenceList = util_list_alloc((int)NULL,NULL);
      DepInfoPtr(node)->ReferenceNumber = RefCount++;
    }
}

static int CreateDepInfo(AST_INDEX node,
			 int       level,
		         SymDescriptor Symtab)

  {
   AST_INDEX name;
   Directive *Dir;
   static Directive *Prefetches[100];

     if (is_comment(node))
       {
	 Dir = new Directive;
	 if (a2i_string_parse(gen_get_text(gen_COMMENT_get_text(node)),Dir,Symtab))
	   {
	     Dir->DependenceList = util_list_alloc((int)NULL,NULL);
	     Dir->StmtNumber = get_stmt_info_ptr(node)->stmt_num;
	     PutDirectiveInfoPtr(node,Dir);
	     switch (Dir->Instr) 
	       {
	       case PrefetchInstruction:
		 assert(Dir->DirectiveNumber < 100);
		 Prefetches[Dir->DirectiveNumber] = Dir;
		 break;
		 
	       case Dependence:
		 assert(Dir->DirectiveNumber < 100);
		 AddDependenceToDirective(Prefetches[Dir->DirectiveNumber],
					  Dir->Subscript);
		 break;
		 
	       case FlushInstruction:
	       case SetSLRInstruction:
		 break;
	       }
	   }
	 else
	   {
	    PutDirectiveInfoPtr(node,NULL);
	    delete Dir;
	   }
       }
     else
       walkIDsInStmt(node,(WK_IDS_CLBACK_V)CreateDepInfoSubscript,Symtab);
     return(WALK_CONTINUE);
  }


static int StoreCacheInfo(AST_INDEX     node,
			  CacheInfoType *CacheInfo)
			  
  {
     if (is_subscript(node))
      {
       DepInfoPtr(node)->Locality = CacheInfo->ReuseModel->GetNodeReuseType(node);

       //
       // Identify the load that should bring in two cache lines
       // to eliminate misses associated with self-spatial loads
       // The load should have self-spatial reuse or group-spatial
       // with the self-spatial property and be a trailer in 
       // group-spatial set.
       //

       if (aiSpecialCache && 
	   NOT(get_subscript_ptr(gen_SUBSCRIPT_get_name(node))->store) &&
	   CacheInfo->ReuseModel->HasSelfSpatialReuse(node))
	 {
	   DepInfoPtr(node)->UseSpecialSelfSpatialLoad = 
	     CacheInfo->ReuseModel->IsGroupSpatialLoadLeader(node);
	   CacheInfo->HasSpecialSelfSpatial = 
	     BOOL(CacheInfo->HasSpecialSelfSpatial
	    	  || DepInfoPtr(node)->UseSpecialSelfSpatialLoad);
	 }
      }
     return(WALK_CONTINUE);
  }


static Boolean IsPrefetch(AST_INDEX Stmt)

{
  if (is_comment(Stmt))
    if (DirectiveInfoPtr(Stmt) != NULL)
      return BOOL(DirectiveInfoPtr(Stmt)->Instr == PrefetchInstruction);
  return false;
}

static Boolean IsDead(AST_INDEX Stmt)

{
  if (is_comment(Stmt))
    if (DirectiveInfoPtr(Stmt) != NULL)
      return BOOL(DirectiveInfoPtr(Stmt)->Instr == FlushInstruction);
  return false;
}

static void AddDependencesToReferences(Directive *Dir,
				       UtilList  *RefList,
				       char      *InnerIvar)
{
  UtilNode *Ref;
  DepStruct *Dep;

  for (Ref = UTIL_HEAD(RefList);
       Ref != NULLNODE;
       Ref = UTIL_NEXT(Ref))
    if (IsValidDependence(Dir->Subscript,(AST_INDEX)UTIL_NODE_ATOM(Ref),InnerIvar))
      {
	Dep = new DepStruct;
	Dep->ReferenceNumber = Dir->DirectiveNumber; 
	Dep->DType = 'c';
	Dep->Distance = 0;
	util_append(DepInfoPtr((AST_INDEX)UTIL_NODE_ATOM(Ref))->DependenceList,
		    util_node_alloc((Generic)Dep,NULL));
      }
}


static int AddRefsToList(AST_INDEX  node,
			 UtilList   *List)

{
  if (is_subscript(node))
    util_append(List,util_node_alloc(node,NULL));
  return(WALK_CONTINUE);
}

static int RemoveRefsFromList(AST_INDEX  Node,
			      UtilList   *List)

{
  UtilNode *ListNode;
  Boolean Found = false;

  if (is_subscript(Node))
    for (ListNode = UTIL_HEAD(List);
	 ListNode != NULLNODE && NOT(Found);
	 ListNode = UTIL_NEXT(ListNode))
      if (UTIL_NODE_ATOM(ListNode) == Node)
	{
	  util_pluck(ListNode);
	  Found = true;
	}
  return(WALK_CONTINUE);
}

static void UpdateDirectiveDependences(Directive *Dir,
				       UtilList  *RefList)
{
  UtilNode *Ref, *Dep;
  DepStruct *Dependence;

  for (Ref = UTIL_HEAD(RefList);
       Ref != NULLNODE;
       Ref = UTIL_NEXT(Ref))
    for (Dep = UTIL_HEAD(Dir->DependenceList);
         Dep != NULLNODE;
         Dep = UTIL_NEXT(Dep))
      {
	Dependence = (DepStruct *)UTIL_NODE_ATOM(Dep);
	if (pt_expr_equal(Dependence->Reference,(AST_INDEX)UTIL_NODE_ATOM(Ref)) &&
	    Dependence->ReferenceNumber == -1)
	  {
	    Dependence->ReferenceNumber = DepInfoPtr(UTIL_NODE_ATOM(Ref))->ReferenceNumber;
	    break;
	  }
      }
}


static void BuildPrefetchDependenceList(CacheInfoType *CacheInfo)

{
  AST_INDEX Stmt,StmtList;
  UtilList *BeforeList,*AfterList;
  char *InnerIvar;

  BeforeList = util_list_alloc((int)NULL,NULL);
  AfterList = util_list_alloc((int)NULL,NULL);
  StmtList = gen_DO_get_stmt_LIST(CacheInfo->loop_data[CacheInfo->loop].node);
  InnerIvar = CacheInfo->IVar[CacheInfo->loop_data[CacheInfo->loop].level-1]; 

  for (Stmt = list_first(StmtList);
       Stmt != AST_NIL;
       Stmt = list_next(Stmt))
    walk_expression(Stmt,(WK_EXPR_CLBACK)AddRefsToList,NOFUNC,(Generic)AfterList);

  for (Stmt = list_first(StmtList);
       Stmt != AST_NIL;
       Stmt = list_next(Stmt))
    {
      if (IsPrefetch(Stmt))
	UpdateDirectiveDependences(DirectiveInfoPtr(Stmt),AfterList);
      else if (IsDead(Stmt))
	AddDependencesToReferences(DirectiveInfoPtr(Stmt),BeforeList,InnerIvar);
      walk_expression(Stmt,(WK_EXPR_CLBACK)AddRefsToList,NOFUNC,(Generic)BeforeList);
      walk_expression(Stmt,(WK_EXPR_CLBACK)RemoveRefsFromList,NOFUNC,(Generic)AfterList);
    }
  util_free_nodes(BeforeList);
  util_free_nodes(AfterList);
  util_list_free(BeforeList);
  util_list_free(AfterList);
}

void GroupSpatialEntry::AddSpatialDependences(AST_INDEX TrailerNode)

{
  GenericListIter GLIter(*NodeList);
  GenericListEntry *GLEntry;
  AST_INDEX Node;
  DepStruct *Dep;
  int TrailerStmtNum = get_stmt_info_ptr(ut_get_stmt(TrailerNode))->stmt_num;

    while ((GLEntry = GLIter()) != NULL)
      {
	Node = GLEntry->GetValue();
	if (Node != TrailerNode)
	  {	
	    Dep = new DepStruct;
	    Dep->ReferenceNumber = DepInfoPtr(Node)->ReferenceNumber;
	    Dep->DType = 'c';
            if (TrailerStmtNum <= get_stmt_info_ptr(ut_get_stmt(Node))->stmt_num)
	      Dep->Distance = 0;
	    else
	      Dep->Distance = 1;
	    util_append(DepInfoPtr(TrailerNode)->DependenceList,
			util_node_alloc((Generic)Dep,NULL));
	  }
      }
}


Boolean DataReuseModelEntry::AddSpatialDependences(AST_INDEX node)
 
{
  GSSetIter GSIter(*gsset);
  GroupSpatialEntry *GSEntry;
  Boolean Found = false;
  AST_INDEX TrailerNode;

  while ((GSEntry = GSIter()) != NULL && NOT(Found))
    {
      if (GSEntry->TrailerNode() == node)
	{
	  GSEntry->AddSpatialDependences(node);
	  Found = true;
	}
    }
  return(Found);
}

void DataReuseModel::AddSpatialDependences(AST_INDEX node)

{
  DRIter ReuseIter(*this);
  DataReuseModelEntry *ReuseEntry;
  Boolean Found = false;

  while ((ReuseEntry = ReuseIter()) != NULL && NOT(Found))
    Found = ReuseEntry->AddSpatialDependences(node);
}      

static int BuildGroupSpatialDependenceList(AST_INDEX     node,
					   DataReuseModel *ReuseModel)
			  
  {
     if (is_subscript(node))
       if (DepInfoPtr(node)->UseSpecialSelfSpatialLoad)
	 ReuseModel->AddSpatialDependences(node);
     return(WALK_CONTINUE);
  }
   
static int LoadCycles(AST_INDEX Node,
		      CacheCycleInfoType *CycleInfo)

  {
    int LoadPenalty = 
      ((config_type *)PED_MH_CONFIG(CycleInfo->ped))->hit_cycles;

    int MissPenalty = 
      ((config_type *)PED_MH_CONFIG(CycleInfo->ped))->miss_cycles;
    
    switch(CycleInfo->ReuseModel->GetNodeReuseType(Node))
       {
	case SELF_TEMPORAL:
	case GROUP_TEMPORAL:
	  return 0;

	case SELF_SPATIAL:
        case GROUP_SPATIAL:
	  return(LoadPenalty);

	case NONE:
	  if (((config_type *)PED_MH_CONFIG(CycleInfo->ped))->NonBlockingCache)
	    return(LoadPenalty);
	  else
	    return(LoadPenalty + MissPenalty);
       }
  }

//
//  Function: OperationCycles
//
//  Input: Node - binary operator AST
//         ped - configuration info
//
//  Output: number of cycles for an operator
//
//  Description: look up in configuration info how long a particular 
//               operation takes
//

static int OperationCycles(AST_INDEX Node,
			   PedInfo   ped)

  {
    int ops;  // complex operations take more than 1 instruction
     
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

//
//  Function: CountCycles
//
//  Input: Node - AST node of operation to check cycle count on
//         CycleInfo - record of cycles in loop so far
//
//  Output: Increase in number of cycles in loop depending on operation
//
//  Description: Add to memory cycles for array references and flops for operators
//

static int CountCycles(AST_INDEX     Node,
		       CacheCycleInfoType *CycleInfo)

  {
    if (is_subscript(Node))
      CycleInfo->IntCycles += LoadCycles(Node,CycleInfo);
    else if (is_binary_op(Node))
      CycleInfo->FPCycles += OperationCycles(Node,CycleInfo->ped);
    return(WALK_CONTINUE);
  }

//
//  Function: CyclesPerIteration
//
//  Input: Node -  AST index of an innermost loop
//         ped - dependence graph and configuration info
//
//  Output: The number of floating-point and memory cycles in a loop
//
//  Description: walk the statements in the AST and compute how much each
//               node requires in machine cycles
//

static int CyclesPerIteration(AST_INDEX Node,
			      DataReuseModel *ReuseModel,
			      int NumberOfAddressSets,
			      PedInfo   ped)

  {
    CacheCycleInfoType CycleInfo; // Cycle information
  
    // Initilization 

    CycleInfo.IntCycles = 0;
    CycleInfo.FPCycles = 0;
    CycleInfo.ped = ped;
    CycleInfo.ReuseModel = ReuseModel;

    // walk statements and compute cycles

    walk_expression(gen_DO_get_stmt_LIST(Node),(WK_EXPR_CLBACK)CountCycles,
		    NOFUNC,(Generic)&CycleInfo);
     
    // conservatively assume that ints and flops are parallel.  This gives a
    // lower bound on cycle time 

    CycleInfo.FPCycles = 
      ceil_ab(CycleInfo.FPCycles,
	      ((config_type *)PED_MH_CONFIG(ped))->FPUnits);
	      
    // add in the integer instructions for address arithmetic
    // and the loop induction variable if there is not an
    // auto-increment address mode

    if (!((config_type *)PED_MH_CONFIG(ped))->AutoIncrement)
      CycleInfo.IntCycles += (NumberOfAddressSets+1); 
    
    CycleInfo.IntCycles = 
      ceil_ab(CycleInfo.IntCycles,
	      ((config_type *)PED_MH_CONFIG(ped))->IntegerUnits);

    if (CycleInfo.IntCycles >= CycleInfo.FPCycles)
      return(CycleInfo.IntCycles);
    else
      return(CycleInfo.FPCycles);
  }

static void AddSpecialLoadDistanceInstruction(AST_INDEX LoopHeader,
	 				      DataReuseModel *ReuseModel,
					      int NumberOfAddressSets,
					      PedInfo ped)
{
  
  // Get the number of cycles for enough loop iterations to go through an
  // an entire cache line

  int LoopCycles = CyclesPerIteration(LoopHeader,ReuseModel,
				      NumberOfAddressSets,ped) * 
    (((config_type *)PED_MH_CONFIG(ped))->line >> 3); 

  // Get how long will it take for the Special Load to fetch the 
  // prefetched cache line

  int PrefetchLatency = ((config_type *) PED_MH_CONFIG(ped))->prefetch_latency;

  // Get how far away in memory the prefetched line is

  int SpecialLoadDistance = ceil_ab(PrefetchLatency,LoopCycles) *
    ((config_type *)PED_MH_CONFIG(ped))->line;

  
  // Build directive to set special load distance 

  char *Instruction = new char[72];

  sprintf(Instruction,"$directive setslr %d",SpecialLoadDistance);

  AST_INDEX NewDirective = pt_gen_comment(Instruction);
  
  Directive *Dir = new Directive;
  Dir->DependenceList = util_list_alloc((int)NULL,NULL);
  Dir->StmtNumber = 0;
  Dir->Instr = SetSLRInstruction;
  Dir->SpecialLoadStride = SpecialLoadDistance;
  PutDirectiveInfoPtr(NewDirective,Dir);

  list_insert_before(LoopHeader,NewDirective);

}


static void walk_loops(CacheInfoType  *CacheInfo,
		       int            loop)
		       

  {
   int i;
   int *LIS;
   UniformlyGeneratedSets *UGS;

     CacheInfo->IVar[CacheInfo->loop_data[loop].level-1] = 
          gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
                       CacheInfo->loop_data[loop].node)));
     if (CacheInfo->loop_data[loop].inner_loop == -1)
       {
	CacheInfo->loop = loop;
	CacheInfo->HasSpecialSelfSpatial = false;
	LIS = new int[CacheInfo->loop_data[loop].level];
	for (i = 0; i < CacheInfo->loop_data[loop].level-1; i++)
	  LIS[i] = 0;
	AST_INDEX step = gen_INDUCTIVE_get_rvalue3(
                          gen_DO_get_control(CacheInfo->loop_data[loop].node));
	if (step == AST_NIL)
	  LIS[CacheInfo->loop_data[loop].level-1] = 1;
	else if (pt_eval(step,&LIS[CacheInfo->loop_data[loop].level-1]))
	  LIS[CacheInfo->loop_data[loop].level-1] = 1;
	UGS = new UniformlyGeneratedSets(CacheInfo->loop_data[loop].node,
					 CacheInfo->loop_data[loop].level,
					 CacheInfo->IVar,LIS);
	CacheInfo->ReuseModel = new DataReuseModel(UGS);
					 
	walk_expression(CacheInfo->loop_data[loop].node,
			(WK_EXPR_CLBACK)StoreCacheInfo,
			NOFUNC,(Generic)CacheInfo);
	walk_expression(CacheInfo->loop_data[loop].node,
			(WK_EXPR_CLBACK)BuildDependenceList,NOFUNC,
			(Generic)CacheInfo);
	BuildPrefetchDependenceList(CacheInfo);
	if (aiSpecialCache)
	  {
	    CacheInfo->AECS =
	      new AddressEquivalenceClassSet(CacheInfo->loop_data[loop].node,
					     CacheInfo->loop_data[loop].level,
					     CacheInfo->IVar);
	    walk_expression(CacheInfo->loop_data[loop].node,
			    (WK_EXPR_CLBACK)BuildGroupSpatialDependenceList,
			    NOFUNC,(Generic)CacheInfo->ReuseModel);
	    if (CacheInfo->HasSpecialSelfSpatial)
	     AddSpecialLoadDistanceInstruction(CacheInfo->loop_data[loop].node,
					       CacheInfo->ReuseModel,
					       CacheInfo->AECS->GetSize(),
					       CacheInfo->ped);
	    delete CacheInfo->AECS;
	  }
	      
	delete CacheInfo->ReuseModel;
	delete UGS;
	delete LIS;
       }
     else
       {
	i = CacheInfo->loop_data[loop].inner_loop;
	while(i != -1)
	  {
	   walk_loops(CacheInfo,i);
	   i = CacheInfo->loop_data[i].next_loop;
	  }
       }
  }


void memory_PerformCacheAnalysis(PedInfo       ped,
				 SymDescriptor symtab,
				 arena_type    *ar,
				 AST_INDEX     root,
				 int           level,
				 int           loop_num)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   CacheInfoType  CacheInfo;

     CacheInfo.loop_data = (model_loop *)ar->arena_alloc_mem_clear
                  (LOOP_ARENA,loop_num*sizeof(model_loop)*4);
     ut_analyze_loop(root,CacheInfo.loop_data,level,ped,symtab);

     walk_statements(root,level,(WK_STMT_CLBACK)CreateDepInfo,NOFUNC,
		     (Generic)symtab);
     CacheInfo.ped = ped;
     CacheInfo.IVar = new char*[loop_num];
     CacheInfo.symtab = symtab;
     CacheInfo.ar = ar;
     walk_loops(&CacheInfo,0);
     delete CacheInfo.IVar;
  }
