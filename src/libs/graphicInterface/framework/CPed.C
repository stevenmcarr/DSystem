/* $Id: CPed.C,v 1.4 2001/09/17 00:39:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*	cped/CPed.h                                                     */
/*                                                                      */
/*	CPed -- abstract Ped engine                                     */
/*	Last edited: November 11, 1993 at 2:40 am			*/
/*                                                                      */
/************************************************************************/




#include <unistd.h>

#include <libs/graphicInterface/framework/CPed.h>

#include <libs/graphicInterface/framework/CFortEditor.h>

#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>
#include <libs/support/arrays/ExtensibleArray.h>
#include <libs/graphicInterface/oldMonitor/include/sms/list_sm.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/frontEnd/ast/groups.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#undef R

#if 1
#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/symtab.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/perf.h>
#endif

#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm.h>

EXTERN( char *,	dg_var_name, (DG_Edge *Edge) ); /* from util_dg.c */






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* variable table */

typedef struct
  {
    PedVariableKind	kind;
    Slist *		entry;

  } VarTableEntry;




/* CPed object */

typedef struct CPed_Repr_struct
  {
    /* creation parameters */
      FortTextTree	ftt;
      FortTree		ft;

      CFortEditor *	editor;

    /* other cped stuff */
      Window *		dummyWindow;

      int		numVars;
      VarTableEntry *	varTable;
      PedVariableOrder	variableOrder;

    /* ped stuff */
      PedInfo		ped;
      
      /* dialogs */
        TransMenu	th;
        Dialog *	fusionDi;
        SharDia		sh;
        InterDia	ih;
        DistrDia	dh;
        SinterDia	sih;
        EdgeDia		eh;
        PerfDia		pm;
        StripDia	smih;
        AllDia		allh;
        Generic		subdia;
	
      /* ptree stuff */
        Company		code_type;
        char *		filename;
      
      /* status */
        Boolean	do_perf_init;

  } CPed_Repr;


#define R(ob)		(ob->CPed_repr)

#define INHERITED	DBObject






/*************************/
/*  Miscellaneous	 */
/*************************/




/* initialization nesting */


static int CPed_initCount = 0;




/* global for sorting variables */

static CPed * cped_cped;
static PedVariableOrder cped_variableOrder;






/*************************/
/*  Forward declarations */
/*************************/




/* ped callbacks */

static FortEditor getFortEditor_local(CPed * cped);

static void treeChanged(CPed * cped, FortTreeNode node);

static void treeWillChange(CPed * cped, FortTreeNode node);

// extern "C" void pedcp_UpdateNodeInfo();

static char * getLine(CPed * cped, FortTreeNode node);

static void selectNode(CPed * cped, FortTreeNode node);

static void redrawSrcSink(CPed * cped,
                          FortTreeNode loop,
                          FortTreeNode src,
                          FortTreeNode sink,
                          FortTreeNode prev_src,
                          FortTreeNode prev_sink);

static void redrawLoop(CPed * cped, FortTreeNode );




/* custom ped interface */

//extern "C"
//{
//  char * ped_DepToText(PedInfo ped, DG_Edge * ea, int edge, int id);
//};




/* ped routines without proper declarations */

//extern "C" void pedReinitialize(PedInfo ped);

//extern "C" Boolean hash_delete(Generic h, int key1, int key2, int key3);

//extern "C" void dt_update(PedInfo ped, FortTreeNode node);




/* monitor routines without proper declarations */



/* misc */

static void makeDummyPanes(CPed * cped,
                           Generic &depTitlePane,
                           Generic &depHeaderPane,
                           Generic &depPane,
                           Point &depSize);

static void deleteDepFiles(CPed * cped);

static void getField(char * buff,  char * &from,  int fieldLen, int skipLen);

static void getStatement(char * from, char * type_buff, char * stmt_buff,
                         char * level_buff);

static void calcVarTable(CPed * cped);

static int sourceLineNum(CPed * cped, FortTreeNode node);

static char * sourceLineNumString(CPed * cped, FortTreeNode node, char * buffer);

static int compareVars(const void * arg1, const void * arg2);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void CPed::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(DBObject);
    REQUIRE_INIT(CFortEditor);
}




void CPed::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(CPed)




CPed::CPed(Context context, DB_FP * session_fd)
    : DBObject(context, session_fd)
{
  /* allocate instance's private data */
    this->CPed_repr = (CPed_Repr *) get_mem(sizeof(CPed_Repr), "CPed instance");
}




CPed::~CPed()
{
  /* discard the ped instance */
    ft_DetachSideArray(R(this)->ft, PED_INFO_SIDE_ARRAY(R(this)->ped));
    xfree((int *) PED_INFO_ARRAY(R(this)->ped));
    pedFinalize(R(this)->ped);

    /*** pedcp_destroy_dialogs(pedcp); ***/

  /* discard miscellaneous stuff */
    /*** what to do about R(this)->dummyWindow? ***/	/*???*/

  free_mem((void*) this->CPed_repr);
}




void CPed::SetEditor(CFortEditor * editor)
{
  R(this)->editor = editor;
}






/*************/
/*  Database */
/*************/




void CPed::GetAttribute(char * &attr)
{
  attr = CPed_Attribute;
}




void CPed::Open(Context context,
		Context mod_in_pgm_context,
		Context pgm_context,
		DB_FP * session_fd,
		FortTextTree ftt)
{
  FortTree ft;
  Generic depTitlePane, depHeaderPane, depPane;
  Point depSize;
  char unixpath[MAXPATHLEN];
  char * sort;
  Boolean has_errors;

  /* open the source module */
    R(this)->ftt = ftt;
    R(this)->ft  = ftt_Tree(ftt);

  /* create the ped instance */
    ft_AstSelect(R(this)->ft);
    ASSERT( ft_Check(R(this)->ft) );

    R(this)->ped =
        (PedInfo) pedInitialize((Generic) this, 
		                 R(this)->ftt, R(this)->ft, 
		                 context,
		                 mod_in_pgm_context,
		                 pgm_context,
		                 false,
		                 false);

    makeDummyPanes(this, depTitlePane, depHeaderPane, depPane, depSize);

    pedRegister(R(this)->ped,
                depTitlePane,
                depHeaderPane,
                depPane,
                depSize,
                (ped_GetFortEditorFunc) getFortEditor_local,
                (ped_TreeChangedFunc) treeChanged,
                (ped_TreeWillChangeFunc) treeWillChange,
                pedcp_UpdateNodeInfo,
                (ped_GetLineFunc) getLine,
                (ped_SelectNodeFunc) selectNode,
                (ped_RedrawSrcSinkFunc) redrawSrcSink,
                (ped_RedrawLoopFunc) redrawLoop);

    el_set_view_lc(PED_EL(R(this)->ped), true);
    el_set_view_control(PED_EL(R(this)->ped), true);
    el_set_view_li(PED_EL(R(this)->ped), true);
    el_set_view_private(PED_EL(R(this)->ped), false);    /* TEMPORARY */

    pedUpdate(R(this)->ped, nil);	/* 'nil' should be saved current loop */

  /* initialize stuff for the ped instance */
    /* this is bogus & doesn't work for multiple instances */
      Global_Dep_Ptr = R(this)->ped;
      R(this)->do_perf_init = true;

    getcwd(unixpath, MAXPATHLEN);
    strcat(unixpath, "/");
    R(this)->filename = ssave(unixpath);

    R(this)->code_type = IBM;

  /* create the ped dialogs */
    /**** pedcp_create_dialogs(pedcp); ****/

  /* initialize the cped variable table */
    R(this)->varTable = nil;
    R(this)->variableOrder = pedVarSortName;
    calcVarTable(this);

  /* establish initial dep sort order */
#ifdef WANT_COMPILER_BUG
    sort = "src";
    el_sort(R(this)->ped, sort);
#endif
}




void CPed::Save(Context context, DB_FP * session_fd)
{
#ifdef NOTDEF
	if (wantSave && NOT(saveAs))
	{
		if (NOT(PED_LOCAL_ANALYSIS(ped)))	/* graph/index files present	*/
		{
			yes_no("Existing dependence file will\nbe deleted.  Save anyway?", 
				&ok, false);

			if (ok)
			{
				delete_dep_file(pedcp); 	/* delete graph/index files	*/
				PED_LOCAL_ANALYSIS(ped) = true;	/* but just delete once		*/
			}
			else
			{
				wantSave = false;
			}
		}
	}

	/*----------------------*/
	/* save if still wanted */

	if (wantSave)
	{
		db_cdesc_get_context(cd, &context, &entity, &type);
		save(pedcp, context);
		db_cdesc_did_save(cd, DBW_VERSION, Sversion_type);

		/* make the new context current if wanted */

		if (NOT(saveCopy))
		{
			R(pedcp)->changed = false;

			if (context != R(pedcp)->context)
			{
				db_cdesc_copy(cd, R(pedcp)->cd);
				R(pedcp)->context = context;
			}
		}

		titleWindow(pedcp);

		/* mark that edits have been saved	*/

		PED_EDIT_SAVED(ped) = true;	
	}
#else
  NOT_CALLED("CPed::Save");
#endif
}









/**************/
/*  Exporting */
/**************/




void CPed::Export(void)
{
#if 0
		if (((PedInfo) (R(pedcp)->ped))->edit_performed && 
			!ed_CheckModule(R(pedcp)->editor))
		{
			message("Module contains errors\nExport is not possible");
			ff_SetShowErrors(R(pedcp)->filter, true);
			return;
		}

		save_dialog_run(pedcp);
#else
  NOT_CALLED("CPed::Export");
#endif
}






/**************************/
/*  Access to dependences */
/**************************/




int CPed::NumDependences(void)
{
  EL_Instance * el = (EL_Instance *) ( PED_EL(R(this)->ped) );

  if( PED_EDIT_PERFORMED(R(this)->ped) )
    return 0;
  else
    return el_total_num_deps(el);
}




void * CPed::GetDependenceEdge(int k)
{
  DG_Edge * ea;
  EL_Instance * el;
  int edge;

  /* locate the requested dependence in ped's structure */
    ea   = dg_get_edge_structure(PED_DG(R(this)->ped));
    el   = (EL_Instance *) PED_EL(R(this)->ped);
    edge = get_dependence(el, k);

  return &ea[edge];
}




void CPed::GetDependence(int k, Dependence &dep)
{
  DG_Edge * edge = (DG_Edge *) this->GetDependenceEdge(k);
  Boolean shared;
  Slist * share_info;
  char *name;

  /* copy the relevant information about the dependence */
    dep.edge  = edge;
    dep.src   = edge->src;
    dep.sink  = edge->sink;
    dep.level = edge->level;
    dep.type  = edge->type;

  /* compute other information about the dependence */
    if( name = dg_var_name(edge) )
      { shared = check_if_shared(PED_LI(R(this)->ped), dep.type, name, &share_info);
        sfree(name);
      }
    else /* can't find name of var for dependence edge */
      { shared = true;
        share_info = NULL;
      }

    dep.loopCarried     = BOOL( dep.level > 0 && shared );
    dep.loopIndependent = BOOL( dep.level == -1 && shared);
    dep.control         = BOOL( dep.type == dg_control );
    dep.xprivate        = false;  /* ??? */
}




void CPed::GetDependenceTexts(int k,
                              char * &type,
                              char * &src,
                              char * &sink,
                              char * &vector,
                              char * &level,
                              char * &block,
                              char * &stmt)
{
  DG_Edge * ea;
  EL_Instance * el;
  int edge;
  char * dep;
  static char type_buff  [20];
  static char src_buff   [20];
  static char sink_buff  [20];
  static char vector_buff[20];
  static char level_buff [20];
  static char block_buff [20];
  static char stmt_buff  [200];

  /* locate the requested dependence in ped's structure */
    ea   = dg_get_edge_structure(PED_DG(R(this)->ped));
    el   = (EL_Instance *) PED_EL(R(this)->ped);
    edge = get_dependence(el, k);

  /* ask ped to format the dependence into a line of text */
    dep = ped_DepToText(R(this)->ped, ea, edge, k);

  /* disassemble ped's line into desired parts */
    switch( ea[edge].type )
      {
        case dg_true:
        case dg_anti:
        case dg_output:
	case dg_input:
        case dg_inductive:
          /* "Induct   %-16s %-16s %-13s independent %-8s" */
          getField(type_buff,   dep,  8, 1);
          getField(src_buff,    dep, 16, 1);
          getField(sink_buff,   dep, 16, 1);
          getField(vector_buff, dep, 13, 1);
          getField(level_buff,  dep, 11, 1);
          getField(block_buff,  dep,  8, 0);
          strcpy(stmt_buff, "");
          break;

        case dg_exit:
        case dg_io:
        case dg_call:
        case dg_control:
        case dg_unknown:
          getStatement(dep, type_buff, stmt_buff, level_buff);
          strcpy(src_buff, "");
          strcpy(sink_buff, "");
          strcpy(vector_buff, "");
          strcpy(block_buff, "");
	  break;
        default:
          die_with_message("CPed.C: unexpected dependence type %i", ea[edge].type);
          break;	
      }

  type   = type_buff;
  src    = src_buff;
  sink   = sink_buff;
  vector = vector_buff;
  level  = (strcmp(level_buff, "independent") == 0 ? (char*)"-" : (char*)level_buff);
  block  = block_buff;
  stmt   = stmt_buff;
}




void CPed::SortDependences(PedDependenceOrder order)
{
  char * sort;

  switch( order )
    {
      case pedDepSortSource:		sort = "src";		break;
      case pedDepSortSink:		sort = "sink";		break;
      case pedDepSortType:		sort = "type";		break;
      case pedDepSortDim:		sort = "dim";		break;
      case pedDepSortBlock:		sort = "block";		break;

      case pedDepSortSourceReverse:	sort = "rsrc";		break;
      case pedDepSortSinkReverse:	sort = "rsink";		break;
      case pedDepSortTypeReverse:	sort = "rtype";		break;
      case pedDepSortDimReverse:	sort = "rdim";		break;
      case pedDepSortBlockReverse:	sort = "rblock";	break;
    }

  el_sort(PED_EL(R(this)->ped), PED_DG(R(this)->ped), sort);
  this->Changed(CHANGE_DEPENDENCE, nil);
}






/************************/
/*  Access to variables */
/************************/




int CPed::NumVariables(void)
{
  if( PED_EDIT_PERFORMED(R(this)->ped) )
    return 0;
  else
    return R(this)->numVars;
}




void CPed::GetVariable(int k, PedVariable &var)
{
  Slist * entry = R(this)->varTable[k].entry;

  var.name      = entry->name;
  var.dim       = entry->dim;
#ifdef NOTDEF /*** g++ 2.2.2 bug ***/
  var.block     = (strcmp(entry->cblock, "_local_") == 0 ? "" : entry->cblock);
#else
  if( strcmp(entry->cblock, "_local_") == 0 )
    var.block   = "";
  else
    var.block   = entry->cblock;
#endif
  var.defBefore = entry->def_before;
  var.useAfter  = entry->use_after;
  var.kind      = R(this)->varTable[k].kind;
  var.user      = entry->user;
}





static char buffer[3][100];

void CPed::GetVariableTexts(int k,
                            char * &name,
                            char * &dim,
                            char * &block,
                            char * &defBefore,
                            char * &useAfter,
                            char * &kind,
                            char * &user)
{
  Slist * entry = R(this)->varTable[k].entry;

  name  = entry->name;

  sprintf(buffer[0], "%d", entry->dim);
  dim = buffer[0];

#ifdef NOTDEF  /*** g++ 2.2.2 bug ***/
  block = (strcmp(entry->cblock, "_local_") == 0 ? "" : entry->cblock);
#else
  if( strcmp(entry->cblock, "_local_") == 0 )
    block = "";
  else
    block = entry->cblock;
#endif

  defBefore = sourceLineNumString(this, entry->def_before, buffer[1]);
  useAfter  = sourceLineNumString(this, entry->use_after,  buffer[2]);

  kind      = (R(this)->varTable[k].kind == pedVarShared ? (char*)"shared" : (char*)"private");

  user      = (entry->user ? (char*)"*" : (char*)"");
}




void CPed::MakeVarShared(int k)
{
  Slist * entry = R(this)->varTable[k].entry;
  FortTreeNode loop = PED_SELECTED_LOOP(R(this)->ped);

  R(this)->varTable[k].kind = pedVarShared;
  entry->user = true;

  treeWillChange(this, loop);
  el_remove_private_var(PED_LI(R(this)->ped), loop, entry->name);
  el_add_shared_var(PED_LI(R(this)->ped), loop, entry);
  forcePedUpdate(R(this)->ped, loop, PED_SELECTION(R(this)->ped));
  treeChanged(this, loop);
  this->Changed(CHANGE_VARIABLE, nil);
  this->Changed(CHANGE_DEPENDENCE, nil);
}




void CPed::MakeVarPrivate(int k)
{
  Slist * entry = R(this)->varTable[k].entry;
  FortTreeNode loop = PED_SELECTED_LOOP(R(this)->ped);

  R(this)->varTable[k].kind = pedVarPrivate;
  entry->user = true;

  treeWillChange(this, loop);
  el_remove_shared_var(PED_LI(R(this)->ped), loop, entry->name);
  el_add_private_var(PED_LI(R(this)->ped), loop, entry);
  forcePedUpdate(R(this)->ped, loop, PED_SELECTION(R(this)->ped));
  treeChanged(this, loop);
  this->Changed(CHANGE_VARIABLE, nil);
  this->Changed(CHANGE_DEPENDENCE, nil);
}




void CPed::SortVariables(PedVariableOrder order)
{
  R(this)->variableOrder = order;
  cped_cped = this;
  cped_variableOrder = order;
  qsort((char *) R(this)->varTable, R(this)->numVars, sizeof(VarTableEntry),
        compareVars);
#if 0
  this->Changed(CHANGE_VARIABLE, nil);
#endif
}






/**********/
/*  Loops */
/**********/




void CPed::SetCurrentLoop(FortTreeNode node)
{
  if( PED_EDIT_PERFORMED(R(this)->ped) )  return;

  if( is_list(node) )  node = list_first(node);

  if( node != PED_SELECTED_LOOP(R(this)->ped) )
    { /* work around ped: it won't figure new loop */
      /*    if it's already seen this selection    */
        PED_SELECTION(R(this)->ped) = AST_NIL;

      pedUpdate(R(this)->ped, node);
      /*** dialogUpdate(pedcp); ***/

      calcVarTable(this);
    }
}




FortTreeNode CPed::GetCurrentLoop(void)
{
  if( PED_EDIT_PERFORMED(R(this)->ped) )
    return AST_NIL;
  else
    return PED_SELECTED_LOOP(R(this)->ped);
}






/***********************/
/* Change notification */
/***********************/




void CPed::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);
  LineEditorChange * ch = (LineEditorChange *) change;
  Generic node;

  /* we only receive changes from the source editor */
 
  /* always note the changed selection */
    R(this)->editor->GetSelectedData(node);
    PED_SELECTION(R(this)->ped) = (FortTreeNode) node;

  if( kind == CHANGE_DOCUMENT )
    { PED_EDIT_PERFORMED(R(this)->ped) = true;
      /* notify that there is no longer a current loop */
        this->Changed(CHANGE_LOOP, (void *) AST_NIL);
    }
}




/* sadly, this routine must duplicate 'analyze_dialog_run' from PEditorCP.c */

void CPed::Analyze(void)
{
  Generic node;

  if( PED_EDIT_PERFORMED(R(this)->ped) )
    if( ! R(this)->editor->CheckData() )
      { message("The module contains errors,\nso analysis is not possible.");
        R(this)->editor->SetShowErrors(true);
      }
    else
      { show_message2("Analyzing dependences...");
        ft_AstSelect(R(this)->ft);
        pedReinitialize(R(this)->ped);
        hide_message2();

        PED_EDIT_PERFORMED(R(this)->ped) = false;

        R(this)->editor->GetSelectedData(node);
        PED_SELECTION(R(this)->ped) = node;
        forcePedUpdate(R(this)->ped,
                       (is_loop(node) ? node : AST_NIL),
                       node);

        R(this)->do_perf_init = true;

        calcVarTable(this);
#if 0
        this->Changed(CHANGE_LOOP, (void *) this->GetCurrentLoop());
#endif
      }
}






/***********************/
/* Access to internals */
/***********************/




void * CPed::getPed(void)
{
  return (void *) R(this)->ped;
}




Generic CPed::getFortEditor(void)
{
  return R(this)->editor->getFortEditor();
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/******************/
/*  Ped callbacks */
/******************/




static
FortEditor getFortEditor_local(CPed * cped)
{
  return cped->getFortEditor();
}




static
void treeChanged(CPed * cped, FortTreeNode node)
{
  cped->Changed(CHANGE_TREE_CHANGED, (void *) node);
}




static
void treeWillChange(CPed * cped, FortTreeNode node)
{
  cped->Changed(CHANGE_TREE_WILL_CHANGE, (void *) node);
}




static
char * getLine(CPed * cped, FortTreeNode node)
{
  int l1, c1, l2, c2;

  (void) ftt_NodeToText(R(cped)->ftt, node, &l1, &c1, &l2, &c2);

  return (char *) ftt_GetTextLine(R(cped)->ftt, l1);
}




static
void selectNode(CPed * cped, FortTreeNode node)
{
  /* nothing */
}




static
void redrawSrcSink(CPed * cped,
                   FortTreeNode loop,
                   FortTreeNode src,
                   FortTreeNode sink,
                   FortTreeNode prev_src,
                   FortTreeNode prev_sink)
{
  /* nothing */
}




static
void redrawLoop(CPed * cped, FortTreeNode loop)
{
  /* nothing */
}






/***********************/
/*  Ped initialization */
/***********************/




extern Generic the_cp_manager_id;      /* sadly, "cp_mgr.c" is hacked to initialize this */


static
void makeDummyPanes(CPed * cped,
               Generic &depTitlePane,
               Generic &depHeaderPane,
               Generic &depPane,
               Point &depSize)
{
  Window * window;
  Generic td;

  /* create a suitable tiling descriptor */
    depTitlePane  = sm_vanilla_get_index();
    depHeaderPane = sm_vanilla_get_index();
    depPane       = sm_list_get_index();
    depSize       = makePoint(100,100);

    td = cp_td_join(
            TILE_UP,
		    (aTilingDesc*)cp_td_pane((Pane**)&depTitlePane, depSize),
        	(aTilingDesc*)cp_td_join(
                             TILE_UP,
                             (aTilingDesc*)cp_td_pane((Pane**)&depHeaderPane, depSize),
                             (aTilingDesc*)cp_td_pane((Pane**)&depPane, depSize)));

  /* create and tile the window */
    window = sm_desk_win_create((aMgrInst*)the_cp_manager_id, (Generic) td,
                                DEF_FONT_ID, false);

  /* window remains not-showing */
    R(cped)->dummyWindow = window;
}




static
void deleteDepFiles(CPed * cped)
{
#ifdef NOTDEF
  ContextDesc * cd;
  Context context;
  Generic  type, entity;
  char suffix[20];
  char path[DB_PATH_LENGTH];

  cd = db_cdesc_alloc();
  db_cdesc_copy(R(cped)->cd /*???*/, cd);

  db_cdesc_get_context(cd, &context, &entity, &type);

  sprintf(suffix, "%d.map", context);
  db_get_path(context, suffix, path);
  unlink(path);

  sprintf(suffix, "%d.graph", context);
  db_get_path(context, suffix, path);
  unlink(path);

  sprintf(suffix, "%d.index", context);
  db_get_path(context, suffix, path);
  unlink(path);

  db_cdesc_free(cd);
#else
  NOT_CALLED("CPed.deleteDepFiles");
#endif
}




static
void getField(char * buff,  char * &from,  int fieldLen, int skipLen)
{
  int k, first, last;

  /* find the blank prefix of the field */
    first = 0;
    while( first < fieldLen && from[first] == ' ' )
      first += 1;

  /* find the blank suffix of the field */
    last = fieldLen-1;
    while( last >= 0 && from[last] == ' ' )
      last -= 1;

  /* copy the good part of the field */
    for( k = first;  k <= last;  k++ )
      buff[k-first] = from[k];
    buff[last-first+1] = '\0';

  /* advance over field and trailing gap */
    from += fieldLen + skipLen;
}




static
void getStatement(char * from,
                  char * type_buff, char * stmt_buff, char * level_buff)
{
  int len = strlen(from);
  int k, m;

  /* copy the initial 'type' field of the string */
    k = 0;
    while( from[k] != ' ' )
      { type_buff[k] = from[k];
        k += 1;
      }
    /* ought to include a following "(...)" here */
    type_buff[k] = '\0';

  /* skip spaces */
    while( from[k] == ' ' )
      k += 1;

  /* copy the nonblank suffix of the string to the level_buff */
    m = len-1;
    while( from[m] != ' ' )
      m -= 1;
    strcpy(level_buff, &from[m+1]);
    len = m+1;

  /* copy the remainder of the string to the stmt_buff */
    m = 0;
    while( k < len )
      { stmt_buff[m] = from[k];
        m += 1;  k += 1;
      }
    stmt_buff[m] = '\0';
}




static
void calcVarTable(CPed * cped)
{
  LI_Instance * LI = PED_LI(R(cped)->ped);
  VarTableEntry * vt;
  Slist * s;
  int k, dummy;

  /* determine the number of variables */
    R(cped)->numVars = el_get_num_shared_vars(LI) + el_get_num_private_vars(LI);

  /* allocate a table of appropriate size */
    if( R(cped)->varTable != nil )
      free_mem((void*) R(cped)->varTable);

    vt = R(cped)->varTable = (VarTableEntry *) get_mem(R(cped)->numVars * sizeof(VarTableEntry),
                                                 "calcVarTable");

  /* fill the table with variable-pointers */
    k = 0;

    s = el_get_first_shared_node(LI, &dummy, ALL_SHARED);
    while( s != nil )
      { vt[k].kind  = pedVarShared;
        vt[k].entry = s;
        k += 1;
        s = el_get_next_shared_node(LI, s, &dummy, ALL_SHARED);
      }

    s = el_get_first_private_node(LI, &dummy, ALL_SHARED);
    while( s != nil )
      { vt[k].kind  = pedVarPrivate;
        vt[k].entry = s;
        k += 1;
        s = el_get_next_private_node(LI, s, &dummy, ALL_SHARED);
      }

  /* sort the table */
    cped->SortVariables(R(cped)->variableOrder);
}




static
int sourceLineNum(CPed * cped, FortTreeNode node)
{
  int lineNum, l1, c1, l2, c2;

  if( node == nil )
    lineNum = UNUSED;
  else
    { (void) ftt_NodeToText(R(cped)->ftt, node, &l1, &c1, &l2, &c2);
      lineNum = l1;
    }

  return lineNum;
}




static
char * sourceLineNumString(CPed * cped, FortTreeNode node, char * buffer)
{
  int lineNum = sourceLineNum(cped, node);

  if( lineNum == UNUSED )
    strcpy(buffer, "");
  else
    sprintf(buffer, "%4d", lineNum+1);

  return buffer;
}




static
int compareVars(const void * arg1, const void * arg2)
{
  VarTableEntry * var1 = (VarTableEntry *) arg1;
  VarTableEntry * var2 = (VarTableEntry *) arg2;
  int result, l1, l2;

# define CF(a,b)  ((a) > (b) ? 1 : (a) < (b) ? -1 : 0)

  switch( cped_variableOrder )
    {
      case pedVarSortName:
        result = strcmp(var1->entry->name, var2->entry->name);
        break;

      case pedVarSortDim:
        result = CF(var1->entry->dim, var2->entry->dim);
        break;

      case pedVarSortBlock:
        result = strcmp(var1->entry->cblock, var2->entry->cblock);
        break;

      case pedVarSortDefBefore:
        l1 = sourceLineNum(cped_cped, var1->entry->def_before);
        l2 = sourceLineNum(cped_cped, var2->entry->def_before);
        if( l1 == UNUSED )  l1 = INFINITY;
        if( l2 == UNUSED )  l2 = INFINITY;
        result = CF(l1, l2);
        break;

      case pedVarSortUseAfter:
        l1 = sourceLineNum(cped_cped, var1->entry->use_after);
        l2 = sourceLineNum(cped_cped, var2->entry->use_after);
        if( l1 == UNUSED )  l1 = INFINITY;
        if( l2 == UNUSED )  l2 = INFINITY;
        result = CF(l1, l2);
        break;

      case pedVarSortKind:
        result = CF((int) var1->kind, (int) var2->kind);
        break;

      default:
        result = 0;
        break;
    }

  return result;
}






/******************************************/
/* Routines called by modified Ped engine */
/******************************************/




extern "C"
void CPed_pedRoot(PedInfo ped)
{
  CPed * cped = (CPed *) (PED_ED_HANDLE(ped));

  PED_ROOT(ped) = ft_Root(R(cped)->ft);
}
