/* $Id: CacheAnalysis.C,v 1.11 1997/03/27 20:22:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/Memoria/include/mh_config.h>
#include <libs/Memoria/include/Memoria_label.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/Memoria/include/bound.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/Memoria/include/header.h>
#include <libs/Memoria/include/mark.h>
#include <libs/Memoria/include/analyze.h>
#include <libs/Memoria/include/mem_util.h>
#include <libs/Memoria/annotate/CacheAnalysis.h>
#include <libs/Memoria/annotate/DirectivesInclude.h>

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
			     util_node_alloc((int)Dep,NULL));
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
      DepInfoPtr(node)->DependenceList = util_list_alloc(NULL,NULL);
      DepInfoPtr(node)->ReferenceNumber = RefCount++;
    }
}


static int CreateDepInfo(AST_INDEX node,
			 int       level,
		         SymDescriptor Symtab)

  {
   AST_INDEX name;
   Directive *Dir;

     if (is_comment(node))
       {
	 Dir = new Directive;
	 if (a2i_string_parse(gen_get_text(gen_COMMENT_get_text(node)),Dir,Symtab))
	   {
	     Dir->DirectiveNumber = RefCount++;
	     Dir->DependenceList = util_list_alloc(NULL,NULL);
	     PutDirectiveInfoPtr(node,Dir);
	   }
	 else
	   delete Dir;
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
       DepInfoPtr(node)->Locality = 
         ut_GetReferenceType(node,CacheInfo->loop_data,CacheInfo->loop,
			     CacheInfo->ped,NULL);
       if (DepInfoPtr(node)->Locality == SELF_SPATIAL)
	 CacheInfo->HasSelfSpatial = true;
      }
     return(WALK_CONTINUE);
  }

static Boolean IsPrefetch(AST_INDEX Stmt)

{
  if (is_comment(Stmt))
    if (DirectiveInfoPtr(Stmt) != NULL)
      return (BOOL(DirectiveInfoPtr(Stmt)->Instr == PrefetchInstruction));
  return false;
}

static Boolean IsDead(AST_INDEX Stmt)

{
  if (is_comment(Stmt))
    if (DirectiveInfoPtr(Stmt) != NULL)
      return (BOOL(DirectiveInfoPtr(Stmt)->Instr == FlushInstruction));
  return false;
}

static Boolean IsValidDependence(AST_INDEX PrefetchNode,
				 AST_INDEX RefNode,
				 char      *InnerIvar)

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
	if (NOT(pt_find_var(Sub1,InnerIvar)) && NOT(pt_find_var(Sub2,InnerIvar)))
	  if (NOT(pt_expr_equal(Sub1,Sub2)))
	    return false;
    }
  return true;
}

static void AddDependencesToDirective(Directive *Dir,
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
	Dep->ReferenceNumber = DepInfoPtr(UTIL_NODE_ATOM(Ref))->ReferenceNumber;
	Dep->DType = 'c';
	Dep->Distance = 0;
	util_append(Dir->DependenceList,util_node_alloc((int)Dep,NULL));
      }
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
		    util_node_alloc((int)Dep,NULL));
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

static void BuildPrefetchDependenceList(CacheInfoType *CacheInfo)

{
  AST_INDEX Stmt,StmtList;
  UtilList *BeforeList,*AfterList;
  char *InnerIvar;

  BeforeList = util_list_alloc(NULL,NULL);
  AfterList = util_list_alloc(NULL,NULL);
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
	AddDependencesToDirective(DirectiveInfoPtr(Stmt),AfterList,InnerIvar);
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
      

static void walk_loops(CacheInfoType  *CacheInfo,
		       int            loop)
		       

  {
   int i;

     CacheInfo->IVar[CacheInfo->loop_data[loop].level-1] = 
          gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
                       CacheInfo->loop_data[loop].node)));
     if (CacheInfo->loop_data[loop].inner_loop == -1)
       {
	CacheInfo->loop = loop;
	walk_expression(CacheInfo->loop_data[loop].node,(WK_EXPR_CLBACK)StoreCacheInfo,
			NOFUNC,(Generic)CacheInfo);
	walk_expression(CacheInfo->loop_data[loop].node,
			(WK_EXPR_CLBACK)BuildDependenceList,NOFUNC,(Generic)CacheInfo);
	BuildPrefetchDependenceList(CacheInfo);
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


void memory_PerformCacheAnalysis(PedInfo      ped,
				 SymDescriptor symtab,
				 arena_type   *ar,
				 AST_INDEX    root,
				 int          level)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   pre_info_type  pre_info;
   CacheInfoType  CacheInfo;

     pre_info.stmt_num = 0;
     pre_info.loop_num = 0;
     pre_info.surrounding_do = -1;
     pre_info.surround_node = AST_NIL;
     pre_info.abort = false;
     pre_info.ped = ped;
     pre_info.symtab = symtab;
     pre_info.ar = ar;
     walk_statements(root,level,(WK_STMT_CLBACK)ut_mark_do_pre,
		     (WK_STMT_CLBACK)ut_mark_do_post,(Generic)&pre_info);
     walk_statements(root,level,(WK_STMT_CLBACK)remove_edges,NOFUNC,(Generic)ped);
     CacheInfo.loop_data = (model_loop *)ar->arena_alloc_mem_clear
                  (LOOP_ARENA,pre_info.loop_num*sizeof(model_loop)*4);
     ut_analyze_loop(root,CacheInfo.loop_data,level,ped,symtab);

     walk_statements(root,level,(WK_STMT_CLBACK)CreateDepInfo,NOFUNC,(Generic)symtab);
     CacheInfo.ped = ped;
     CacheInfo.IVar = new char*[pre_info.loop_num];
     CacheInfo.symtab = symtab;
     CacheInfo.ar = ar;
     walk_loops(&CacheInfo,0);
     delete CacheInfo.IVar;
  }
