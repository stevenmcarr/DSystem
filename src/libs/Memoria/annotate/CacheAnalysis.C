/* $Id: CacheAnalysis.C,v 1.34 2000/05/16 18:49:51 carr Exp $ */
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
extern int aiParseComments;
extern int aiLongIntegers;
extern int aiDoubleReals;

static int RefCount = 0;

Boolean IsConstantStride(AST_INDEX Node,char *IVar)
{
  Boolean   Linear, 
            StrideIsConstant = true, 
            AlreadyInASubscript = false;
  int       Coeff;
  AST_INDEX SubList,
            Sub;

  SubList = gen_SUBSCRIPT_get_rvalue_LIST(Node);
  for (Sub = list_first(SubList);
       Sub != AST_NIL && StrideIsConstant;
       Sub = list_next(Sub))
  {
    pt_get_coeff(Sub,IVar,&Linear,&Coeff);
    if (Coeff != 0)
      if (AlreadyInASubscript)
	StrideIsConstant = false;
      else
      {
	AlreadyInASubscript = true;
	StrideIsConstant = Linear;
      }
    else 
      StrideIsConstant = Linear;
  }

  return StrideIsConstant;
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
      DepInfoPtr(node)->UsePrefetchingLoad = false;
      DepInfoPtr(node)->Locality = UNDEFINED;
      DepInfoPtr(node)->AddressLeader = AST_NIL;
      DepInfoPtr(node)->FirstInLoop = AST_NIL;
      DepInfoPtr(node)->Offset = -1;
      DepInfoPtr(node)->StmtNumber = -1;
      DepInfoPtr(node)->PrefetchDistance = -1;
      DepInfoPtr(node)->PrefetchOffsetAST = AST_NIL;
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
	 if (aiParseComments &&
	     a2i_string_parse(gen_get_text(gen_COMMENT_get_text(node)),Dir,Symtab))
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

      // Set the log2 of the max words in a cache line. This is used for prefetch
      // distance.
      
      if (gen_get_real_type(node) == TYPE_DOUBLE_PRECISION ||
	  (gen_get_real_type(node) == TYPE_INTEGER && aiLongIntegers) ||
	  (gen_get_real_type(node) == TYPE_REAL && aiDoubleReals))
        if (CacheInfo->LogMaxBytesPerWord < 3)
	  CacheInfo->LogMaxBytesPerWord = 3;
	else;
      else if (CacheInfo->LogMaxBytesPerWord < 2)
	CacheInfo->LogMaxBytesPerWord = 2;
	
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
	DepInfoPtr(node)->UsePrefetchingLoad = 
	  CacheInfo->ReuseModel->IsGroupSpatialLoadLeader(node);
      else if (DepInfoPtr(node)->Locality == NONE && 
	       IsConstantStride(node,*(CacheInfo->IVar)))

	// Add support for constant stride, non self-spatial referances.

	DepInfoPtr(node)->UsePrefetchingLoad = true;
      else
	DepInfoPtr(node)->UsePrefetchingLoad = false;
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
       if (DepInfoPtr(node)->UsePrefetchingLoad)
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
	  return 0;

	case GROUP_TEMPORAL:
	  if (NOT(get_subscript_ptr(gen_SUBSCRIPT_get_name(Node))->store))
	    return 0;
	  else
	    return LoadPenalty;

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
      {
	CycleInfo->IntCycles += LoadCycles(Node,CycleInfo);
	
	return(WALK_SKIP_CHILDREN);
      }
    else if (is_binary_op(Node))
      if (gen_get_real_type(Node) == TYPE_INTEGER)
	CycleInfo->IntCycles++;
      else
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

// The size of a dimension is ub - lb + 1. If they are both constants
// it is easy to compute. If we have an AST then use
//
//        pt_gen_add(pt_gen_sub(ub,lb),pt_gen_int(1))
//
// use tree_copy_with_type(AST_INDEX) to make a copy of the ASTs returned 
// from GetArrayBound so that we don't have links into the ASTs stored
// in the symbol table (bad idea, dude). The copy routine returns an AST_INDEX.
// to generate an AST for a constant use pt_gen_int(int). You will need to pass
// an AST to the f2i code generation routines to generate the expressions to
// compute symbolic array bound sizes so that the we have a register to use
// in the PFLD instruction. If you have a constant, we can just 
// generate a PFLDI instruction and put the constant value directly in the
// instruction. When you get to the point of needing to have f2i generate the
// proper ILOC, let me know. At that time I'll figure out what needs to be
// done. For now, a constant offset goes in
//     
//        DepInfoPtr(node)->PrefetchingDistance
//
// where node is a subscript node int the AST (is_subscript(node)). An AST
// offset goes in
//
//        DepInfoPtr(node)->PrefetchOffsetAST
//
// If the offset is a constant, set PrefetchOffsetAST to AST_NIL.

void GetArrayBound(SymDescriptor   Symtab,
		   char*           ArrayName,
		   int             SubscriptPosition, // zero-based
                   ArrayBoundEnum& UpperBoundType,
                   ArrayBoundEnum& LowerBoundType,
		   int&            UpperBoundConstValue,
		   int&            LowerBoundConstValue,
		   AST_INDEX&      UpperBoundASTValue,
		   AST_INDEX&      LowerBoundASTValue)

{
 ArrayBound *BoundsInformation = (ArrayBound *)fst_GetField(Symtab,ArrayName,
                                                            SYMTAB_DIM_BOUNDS);

 if (fst_bound_is_const_ub(BoundsInformation[SubscriptPosition]))
   {
    UpperBoundType = constant;
    UpperBoundConstValue = 
	BoundsInformation[SubscriptPosition].ub.value.const_val;
   }
 else
   {
    UpperBoundType = symbolic_expn_ast_index;
    UpperBoundASTValue = 
	BoundsInformation[SubscriptPosition].ub.value.ast;
   }

 if (fst_bound_is_const_lb(BoundsInformation[SubscriptPosition]))
   {
    LowerBoundType = constant;
    LowerBoundConstValue = 
	BoundsInformation[SubscriptPosition].lb.value.const_val;
   }
 else
   {
    LowerBoundType = symbolic_expn_ast_index;
    LowerBoundASTValue = 
	BoundsInformation[SubscriptPosition].lb.value.ast;
   }

}


// Utility functions for creating valid ast_node stuff
AST_INDEX MakeIntConstant(int con)
{
    AST_INDEX p;
    
    p = pt_gen_int(con);
    gen_put_real_type(p, TYPE_INTEGER);
    gen_put_converted_type(p, TYPE_INTEGER);

    return p;
}

AST_INDEX MakeMul(AST_INDEX n1, AST_INDEX n2)
{
    AST_INDEX p;
    
    p = pt_gen_mul(n1, n2);
    gen_put_real_type(p, TYPE_INTEGER);
    gen_put_converted_type(p, TYPE_INTEGER);

    return p;
}

AST_INDEX MakeAdd(AST_INDEX n1, AST_INDEX n2)
{
    AST_INDEX p;
    
    p = pt_gen_add(n1, n2);
    gen_put_real_type(p, TYPE_INTEGER);
    gen_put_converted_type(p, TYPE_INTEGER);

    return p;
}

AST_INDEX MakeSub(AST_INDEX n1, AST_INDEX n2)
{
    AST_INDEX p;
    
    p = pt_gen_sub(n1, n2);
    gen_put_real_type(p, TYPE_INTEGER);
    gen_put_converted_type(p, TYPE_INTEGER);

    return p;
}

void CalcPrefetchingLoadDistance(AST_INDEX Node,
                                 SymDescriptor Symtab,
                                 char* IVar,
                                 int LogMaxBytesPerWord,
                                 AST_INDEX& ASTDistance,
                                 int& ConstVal,
                                 Boolean& FirstSub)
{
    // LOTS of variables for the PFLD stuff.
    AST_INDEX ArrayNameAST,
        SubList,
        Sub,
        UpperBoundAST,
        LowerBoundAST;
    int Coeff,
        Which = 0,
        UpperBoundConst,
        LowerBoundConst;
    Boolean ASTValue = false,
        Linear,
        Done = false;
    ArrayBoundEnum UpperBoundType,
        LowerBoundType;
    char *ArrayName;
    
    FirstSub = true;
    
    ASTDistance = AST_NIL;
    ConstVal = 1;
    
    ArrayNameAST = gen_SUBSCRIPT_get_name(Node);
    ArrayName = gen_get_text(ArrayNameAST);
    
    
    // New stuff (MJB)
    SubList = gen_SUBSCRIPT_get_rvalue_LIST(Node);

    for (Sub = list_first(SubList);
         (Sub != AST_NIL) && (Done != true);
         Sub = list_next(Sub))
    {
        pt_get_coeff(Sub, IVar, &Linear, &Coeff);

        if (Coeff != 0)
        {
            
            if (ASTValue)
            {
                // Tack it on to the AST.
                ASTDistance = MakeMul(MakeIntConstant(Coeff), ASTDistance);
            }
            else
            {

                ConstVal *= Coeff;
            }
            Done = true;
            
        }
        else
        {
            // It is not the first subscript.
            FirstSub = false;
            
            // Find the bounds. I hope that the list and the number are the same...
            GetArrayBound(Symtab, ArrayName, Which, UpperBoundType, LowerBoundType,
                          UpperBoundConst, LowerBoundConst, UpperBoundAST, 
                          LowerBoundAST);

            // First AST value? If so, convert const to AST.
            if (ASTValue == false)
            {
                if (UpperBoundType == symbolic_expn_ast_index ||
                    LowerBoundType == symbolic_expn_ast_index)
                {
                
                    ASTDistance = MakeIntConstant(ConstVal);
                    ASTValue = true;
                }
            }
        
            // Calc it
            if (ASTValue == true)
            {
                // We need to convert everything to ast.
                if (UpperBoundType == constant)
                {
                    UpperBoundAST = MakeIntConstant(UpperBoundConst);
                }
                else
                {
                    // Do the tree copy thing.
                    UpperBoundAST = tree_copy_with_type(UpperBoundAST);
                }
            
            
                if (LowerBoundType == constant)
                {
                    LowerBoundAST = MakeIntConstant(LowerBoundConst);
                }
                else
                {
                    // Do the tree copy thing.
                    LowerBoundAST = tree_copy_with_type(LowerBoundAST);   
                }
            
                // I know, Ug...
                ASTDistance = MakeMul( MakeAdd( MakeSub(UpperBoundAST, 
                                                        LowerBoundAST),
                                                MakeIntConstant(1)),
                                       ASTDistance);

            }
            else
            {
                ConstVal *= (UpperBoundConst - LowerBoundConst + 1);
            }
        }
        
        Which++;
    }

    // Have to mul by the size of an element.
    int SizeOfElement = 1 << LogMaxBytesPerWord;

    if (ASTValue == false)
    {
        ConstVal *= SizeOfElement;
    }
    else
    {
        ASTDistance = MakeMul(ASTDistance, MakeIntConstant(SizeOfElement));
    }
    
                
}


static int SetDistance(AST_INDEX Node,
		       PrefetchDataType *PrefetchData)

{
    AST_INDEX ASTDistance;
    int ConstVal;
    Boolean FirstSub;
    
    if (is_subscript(Node))
    {
        if (DepInfoPtr(Node)->UsePrefetchingLoad)
        {
             
	    assert(DepInfoPtr(Node)->Locality != UNDEFINED);
            if (DepInfoPtr(Node)->Locality != NONE)
            {                 

	      //
	      // Compute how many iterations of the loop are necessary to hide
	      // the prefetch latency. Then, set the prefetch distance to be that
	      // many cache lines ahead + 1 because we are not counting the current
	      // iteration (prefetching load may be at end of loop)
	      // 
	      // We assume the loop has been unrolled, so we are doing a prefetching
	      // load on the last reference in a line. This removes the problem
	      // with cache-line alignment
	      //

	      DepInfoPtr(Node)->PrefetchDistance =
		(ceil_ab(PrefetchData->PrefetchLatency,PrefetchData->LoopCycles) + 1) *
		PrefetchData->LineSize;

	      DepInfoPtr(Node)->PrefetchOffsetAST = AST_NIL;
            }
            else
            {


                // Oops.. Keep in mind that this function calculates the distance
                // between array(...,IVar,...) and array(...,IVar+1,...). MJB.
                CalcPrefetchingLoadDistance(Node, PrefetchData->symtab,
                                            PrefetchData->IVar,
                                            PrefetchData->LogMaxBytesPerWord,
                                            ASTDistance,
                                            ConstVal,
                                            FirstSub);
                
                // the value we have now needs to be multiplied by the number of
                // iterations till we need the data. I don't fiddle with rounding
                // to the size of a cache line, because we want EXACTLY the location.
                // We are most likely jumping through memory with a large stride.
                // The exception is when FirstSub is true, which means the first 
                // subscript was the IVar. Probably it is non Uniformly Generated, so
                // it ended being Locality=NONE. Assume that it is constant stride if
                // we got here.
                if (ASTDistance != AST_NIL)
                {
                    DepInfoPtr(Node)->PrefetchOffsetAST = 
                        MakeMul(ASTDistance, 
                                MakeIntConstant((ceil_ab(PrefetchData->PrefetchLatency, 
                                                         PrefetchData->LoopCycles))));
                    DepInfoPtr(Node)->PrefetchDistance = -1;
                }
                else
                {
                    DepInfoPtr(Node)->PrefetchOffsetAST = AST_NIL;
                    DepInfoPtr(Node)->PrefetchDistance = ConstVal * 
                        (ceil_ab(PrefetchData->PrefetchLatency, PrefetchData->LoopCycles));
                    
                    if (FirstSub == true)
                    {
                        // Round to the next cache line.
                        DepInfoPtr(Node)->PrefetchDistance = ceil_ab(DepInfoPtr(Node)->PrefetchDistance,
                                                                     PrefetchData->LineSize) *
                            PrefetchData->LineSize;
                    }
                    
                        
                }    
            }
             
                 
        }
        else
            DepInfoPtr(Node)->PrefetchDistance = 0;
    }
    return (WALK_CONTINUE);
}

// Modified for prefetching load (non self-spacial).
static void SetPrefetchLoadDistance(AST_INDEX LoopHeader,
				    DataReuseModel *ReuseModel,
				    int NumberOfAddressSets,
				    PedInfo ped,
                                    char *IVar,
				    int LogMaxBytesPerWord,
                                    SymDescriptor symtab)
{
  PrefetchDataType PrefetchData;
  
  // Get the number of cycles for enough loop iterations to go through an
  // an entire cache line

  PrefetchData.LoopCycles = CyclesPerIteration(LoopHeader,ReuseModel,
					       NumberOfAddressSets,ped);

  // Get how long will it take for the Special Load to fetch the 
  // prefetched cache line

  PrefetchData.PrefetchLatency = 
	((config_type *) PED_MH_CONFIG(ped))->prefetch_latency;


  PrefetchData.LineSize = ((config_type *) PED_MH_CONFIG(ped))->line;

  // New stuff (MJB)
  PrefetchData.IVar = IVar;
  PrefetchData.symtab = symtab;
  PrefetchData.LogMaxBytesPerWord = LogMaxBytesPerWord;

  walk_expression(gen_DO_get_stmt_LIST(LoopHeader),(WK_EXPR_CLBACK)SetDistance,
		  (WK_EXPR_CLBACK)NOFUNC,(Generic)&PrefetchData);

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

            // Need the symbol table now, and the IVar. (PFLD)
	    SetPrefetchLoadDistance(CacheInfo->loop_data[loop].node,
				    CacheInfo->ReuseModel,
				    CacheInfo->AECS->GetSize(),
				    CacheInfo->ped,
                                    CacheInfo->IVar[CacheInfo->loop_data[loop].level-1],
				    CacheInfo->LogMaxBytesPerWord,
                                    CacheInfo->symtab);
        
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
     CacheInfo.LogMaxBytesPerWord = 0;
     walk_loops(&CacheInfo,0);
     delete CacheInfo.IVar;
  }
