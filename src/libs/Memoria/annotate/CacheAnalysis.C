/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <general.h>
#include <mh.h>
#include <mh_ast.h>
#include <mh_config.h>
#include <label.h>
#include <pt_util.h>
#include <bound.h>
#include <fort/walk.h>
#include <header.h>
#include <mark.h>
#include <analyze.h>
#include <mem_util.h>
#include <CacheAnalysis.h>
#include <UniformlyGeneratedSets.h>
#include <../uj/do_unroll.h>

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

static void UpdateCopies(AST_INDEX *copies,
			 int       NumCopies,
			 LocalityType Locality)

  {
   int i;

     for (i = 0; i < NumCopies; i++)
       {
	CreateDepInfoPtr(tree_out(copies[i]));
	DepInfoPtr(tree_out(copies[i]))->Locality = Locality;
       }
  }


static int InitializeDepInfo(AST_INDEX Node,
			     Generic NumCopies)

 {
  AST_INDEX name;

    if (is_subscript(Node))
      {
       name = gen_SUBSCRIPT_get_name(Node);
       UpdateCopies(get_subscript_ptr(name)->copies,NumCopies,
		    DepInfoPtr(Node)->Locality);
       if (DepInfoPtr(Node)->Locality == SELF_SPATIAL)
	 DepInfoPtr(Node)->Locality = NONE;
      }
    return(WALK_CONTINUE);
 }

static void UpdateReferences(AST_INDEX List,
			     AST_INDEX Last,
			     int       NumCopies)

  {
   AST_INDEX Stmt;

     for (Stmt = list_first(List);
	  Stmt != list_next(Last);
	  Stmt = list_next(Stmt))
	walk_expression(Stmt,(WK_EXPR_CLBACK)InitializeDepInfo,NOFUNC,NumCopies);
  }
  
static void CreatePostLoop(model_loop    *loop_data,
			   int           loop,
			   int           UnrollVal,
			   PedInfo       ped,
			   SymDescriptor Symtab,
			   arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
  

  {
   AST_INDEX control,new_loop,next,
             lwb,upb,step;
   Boolean   need_pre_loop = false;
   int       lwb_v,upb_v,step_v;
   copy_info_type copy_info;

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
        copy_info.ar = ar;
        copy_info.val = 1;
        copy_info.symtab = Symtab;
        walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
                        (WK_EXPR_CLBACK) ut_init_copies,
                        (WK_EXPR_CLBACK)NOFUNC,(Generic)&copy_info);
	new_loop = ut_tree_copy_with_type(loop_data[loop].node,0,ar);
        set_level_vectors(gen_DO_get_stmt_LIST(loop_data[loop].node),
                          gen_DO_get_stmt_LIST(new_loop),ped);
        walk_expression(loop_data[loop].node,(WK_EXPR_CLBACK)mh_copy_edges,
                        (WK_EXPR_CLBACK)NOFUNC, (Generic)ped);
	ut_update_labels(new_loop,Symtab);
	ut_update_bounds_post(loop_data[loop].node,new_loop,UnrollVal);
	UpdateReferences(list_first(gen_DO_get_stmt_LIST(loop_data[loop].node)),
			 list_last(gen_DO_get_stmt_LIST(loop_data[loop].node)),1);
	list_insert_after(loop_data[loop].node,new_loop);
       }
     else 
       ut_update_bounds_post(loop_data[loop].node,AST_NIL,UnrollVal);
  }

static void UnrollLoop(model_loop   *loop_data,
		       int           loop,
		       int           LineDistance,
		       AST_INDEX     Var,
		       PedInfo	     ped,
		       SymDescriptor Symtab,
		       arena_type    *ar)
  {
   AST_INDEX Stmt,LastStmt,Step;
   int       i;
   copy_info_type copy_info;

     Step = gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(loop_data[loop].node));
     CreatePostLoop(loop_data,loop,LineDistance-1,ped,Symtab,ar);
     LastStmt = list_last(gen_DO_get_stmt_LIST(loop_data[loop].node));
     copy_info.val = LineDistance-1;
     copy_info.symtab = Symtab;
     copy_info.ar = ar;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
		     (WK_EXPR_CLBACK) ut_init_copies,(WK_EXPR_CLBACK)NOFUNC,
		     (Generic)&copy_info);
     mh_replicate_body(gen_DO_get_stmt_LIST(loop_data[loop].node),
		       LineDistance-1,loop_data[loop].level, gen_get_text(Var),
		       Step,ped,Symtab,false,
		       get_stmt_info_ptr(loop_data[loop].node)->loop_num,
		       loop_data[loop].node,gen_get_text(Var),
		       loop_data[loop].level,ar);
     UpdateReferences(gen_DO_get_stmt_LIST(loop_data[loop].node),LastStmt,
		      LineDistance-1);      
  }

static int BuildDependenceList(AST_INDEX node,CacheInfoType *CacheInfo)

  {
   DG_Edge    *dg;
   int        vector;
   EDGE_INDEX edge;
   AST_INDEX  name;
   DepStruct  *Dep;
   int        InnerLevel;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	DepInfoPtr(node)->DependenceList = util_list_alloc(NULL,NULL);
	if (CacheInfo->loop_data[get_subscript_ptr(name)->surrounding_do].inner_loop 
	    != -1)
	  return(WALK_CONTINUE);
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
	     {
	      Dep = new DepStruct;
	      Dep->ReferenceNumber =DepInfoPtr(tree_out(dg[edge].sink))->ReferenceNumber;
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

static int CreateDepInfo(AST_INDEX node,
		         Generic dummy)

  {
   AST_INDEX name;

     if (is_subscript(node))
       CreateDepInfoPtr(node);
     return(WALK_CONTINUE);
  }


static int SetReferenceNumber(AST_INDEX node,
			      CacheInfoType *CacheInfo)

  {
   AST_INDEX name;

     if (is_subscript(node))
       DepInfoPtr(node)->ReferenceNumber = CacheInfo->RefNum++;
     return(WALK_CONTINUE);
  }

static int StoreCacheInfo(AST_INDEX     node,
			  CacheInfoType *CacheInfo)
			  
  {
     if (is_subscript(node))
      {
       DepInfoPtr(node)->Locality = 
         ut_GetReferenceType(node,CacheInfo->loop_data,CacheInfo->loop,
			     CacheInfo->ped,CacheInfo->UGS);
       if (DepInfoPtr(node)->Locality == SELF_SPATIAL)
	 CacheInfo->HasSelfSpatial = true;
      }
     return(WALK_CONTINUE);
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
	if (mc_extended_cache)
	  CacheInfo->UGS=
	      new UniformlyGeneratedSets(CacheInfo->loop_data[loop].node,
					 CacheInfo->loop_data[loop].level,
					 CacheInfo->IVar);
	else
	  CacheInfo->UGS = NULL;
	CacheInfo->HasSelfSpatial = false;
	walk_expression(CacheInfo->loop_data[loop].node,
			(WK_EXPR_CLBACK)StoreCacheInfo,NOFUNC,
			(Generic)CacheInfo);
	if (aiCache > 1 && CacheInfo->HasSelfSpatial)
	  UnrollLoop(CacheInfo->loop_data,loop,
		     ((config_type *)PED_MH_CONFIG(CacheInfo->ped))->line >> 3,
		     gen_INDUCTIVE_get_name(
		        gen_DO_get_control(CacheInfo->loop_data[loop].node)),
		     CacheInfo->ped,CacheInfo->symtab,CacheInfo->ar);
	  
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
   static int RefCount = 0;

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

     CacheInfo.ped = ped;
     CacheInfo.RefNum = RefCount;
     walk_expression(root,(WK_EXPR_CLBACK)CreateDepInfo,NOFUNC,
		     (Generic)NULL);
     RefCount = CacheInfo.RefNum;
     CacheInfo.IVar = new char*[pre_info.loop_num];
     CacheInfo.symtab = symtab;
     CacheInfo.ar = ar;
     walk_loops(&CacheInfo,0);
     delete CacheInfo.IVar;
     walk_expression(root,(WK_EXPR_CLBACK)SetReferenceNumber,NOFUNC,
		     (Generic)&CacheInfo);
     walk_expression(root,(WK_EXPR_CLBACK)BuildDependenceList,NOFUNC,
		     (Generic)&CacheInfo);
  }
